#ifndef POLL_FD_HPP
#define POLL_FD_HPP

#include "../Server.hpp"
#include "../ServerStreams/HttpRequest.hpp"
#include "../ServerStreams/HttpResponse.hpp"
#include "../ServerStreams/cgi/CgiRequest.hpp"
#include "../ServerStreams/cgi/CgiResponse.hpp"
#include "../ServerStreams/proxy/ProxyRequest.hpp"
#include "../ServerStreams/proxy/ProxyResponse.hpp"
#include "../blocks/Block.hpp"
#include "../utils/SharedPtr.hpp"
#include "../webserv.hpp"

typedef int (*clientPollHandlerType)(ClientPoll *);
typedef int (*pollFdHandlerType)(PollFd *);
typedef int (*CgiPollHandlerType)(CgiPoll *);
typedef int (*ProxyPollHandlerType)(ProxyPoll *);

int handleNewConnection(PollFd *listen);
int checkTimeout(time_t time, int seconds);

int checkProxyPoll(ClientPoll *client);
int checkCgiPoll(ClientPoll *client);
int sendResponseToClient(ClientPoll *client);
int executeClientRequest(ClientPoll *client);
int readClientRequest(ClientPoll *client);
int timeoutClient(ClientPoll *client);

int cgiQuitPoll(CgiPoll *cgi);
int waitCgiProcessEnd(CgiPoll *cgi);
int readCgiResponse(CgiPoll *cgi);
int sendCgiRequest(CgiPoll *cgi);
int handleCgiResponse(CgiPoll *cgi);

int timeoutProxy(ProxyPoll *proxy);
int sendProxyRequest(ProxyPoll *proxy);
int recvProxyResponse(ProxyPoll *proxy);

class PollFd {
   protected:
    pollFdHandlerType _readHandler;
    pollFdHandlerType _writeHandler;
    SharedPtr _status;
    Server &_server;
    time_t _startTime;
    int _fd;
    bool _concurrentReadWrite;
    bool _removeOnHungUp;

   public:
    PollFd(int fd, Server &server);
    void destroy(int socketStatus);
    virtual ~PollFd();

    SharedPtr &getStatus();
    void setStatus(int value) {
        *_status = value;
    }
    int getFd();
    bool removeOnHungUp();
    bool isConcurrentReadWrite();
    void setConcurrentReadWrite(bool value);
    void setRemoveOnHungUp(bool value);
    void resetStartTime();
    Server &server();
    time_t startTime();
    void setWriteHandler(pollFdHandlerType f);
    void setReadHandler(pollFdHandlerType f);
    virtual int handleWrite(PollFd *pollFd);
    virtual int handleRead(PollFd *pollFd);
};

class ClientPoll : public PollFd {
   private:
    clientPollHandlerType _writeHandler;
    clientPollHandlerType _readHandler;
    HttpRequest *_req;
    HttpResponse *_res;
    SharedPtr _cgiPollStatus;
    SharedPtr _proxyStatus;
    LocationBlock *_location;

   public:
    ClientPoll(int fd, Server &server);
    ClientPoll(const ClientPoll &other);
    ClientPoll &operator=(const ClientPoll &other);
    ~ClientPoll();

    HttpResponse &res();
    HttpRequest &req();

    void setLocation(LocationBlock &loc);
    int proxyPollStatus();
    int cgiPollStatus();
    void resetConnection();
    void setProxyStatus(SharedPtr &proxyStatus);
    void setCgiPollStatus(SharedPtr &cgiStatus);

    void loadErrorPageFromLocation(LocationBlock *location, int statusCode);
    void setWriteHandler(clientPollHandlerType f);
    void setReadHandler(clientPollHandlerType f);
    int handleWrite(PollFd *pollFd);
    int handleRead(PollFd *pollFd);
    int sendErrorPage(int statusCode);
};

typedef struct CgiSockets {
    int request[2];
    int response[2];
} CgiSockets;

class CgiPoll : public PollFd {
   private:
    CgiPollHandlerType _readHandler;
    CgiPollHandlerType _writeHandler;
    struct pollfd &_pollfd;
    ClientPoll &_client;
    SharedPtr _clientStatus;
    CgiRequest *_cgiReq;
    CgiResponse *_cgiRes;
    CgiSockets _cgiSockets;
    int _pid;

   public:
    CgiPoll(CgiSockets &cgiSockets, ClientPoll &client, struct pollfd &structPollfd);
    ~CgiPoll();

    int cgiPid();
    int tryFork();
    void killCgiProcess();
    void closeRequestSockets();

    CgiRequest &cgiReq();
    CgiResponse &cgiRes();
    int clientStatus();
    ClientPoll &client();
    void forkAndexecuteScript(Server &server, const String &cgiResourcePath, const String &cgiScriptCommand);
    void redirectCgiProcessInputOutput();

    void switchToResponseReadableSocket();
    void switchToRequestWritableSocket();

    void setCgiPid(int value) {
        _pid = value;
    }
    void setReadHandler(CgiPollHandlerType f);
    void setWriteHandler(CgiPollHandlerType f);
    int handleRead(PollFd *pollFd);
    int handleWrite(PollFd *pollFd);
};

class ProxyPoll : public PollFd {
   private:
    ProxyPollHandlerType _readHandler;
    ProxyPollHandlerType _writeHandler;
    SharedPtr _clientStatus;
    ClientPoll &_client;
    ProxyRequest _proxyReq;
    ProxyResponse _proxyRes;

   public:
    ProxyPoll(int socket, ClientPoll &client, ProxyUrl &proxyPass);
    ~ProxyPoll();

    ProxyRequest &proxyReq();
    ProxyResponse &proxyRes();
    int clientStatus();
    ClientPoll &client();
    void setReadHandler(ProxyPollHandlerType f);
    void setWriteHandler(ProxyPollHandlerType f);
    int handleRead(PollFd *pollFd);
    int handleWrite(PollFd *pollFd);
};

#endif