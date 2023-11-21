#ifndef POLL_FD_HPP
#define POLL_FD_HPP

#include "../HttpRequest.hpp"
#include "../HttpResponse.hpp"
#include "../Server.hpp"
#include "../webserv.hpp"

typedef int (*clientPollHandlerType)(Server &, ClientPoll *);
typedef int (*pollFdHandlerType)(Server &, PollFd *);
typedef int (*CgiPollHandlerType)(Server &, CgiPoll *);

int handleNewConnection(Server &server, PollFd *listen);
int checkTimeout(time_t time, int seconds);

// int setCgiResponse(Server &server, ClientPoll *client);
int sendResponseToClient(Server &server, ClientPoll *client);
int executeClientRequest(Server &server, ClientPoll *client);
int readClientRequest(Server &server, ClientPoll *client);
int timeoutClient(Server &server, ClientPoll *client);

int quitPollError(Server &server, CgiPoll *cgi);
int checkCgiPoll(Server &server, ClientPoll *client);
int waitCgiProcessEnd(Server &server, CgiPoll *cgi);
int readCgiResponse(Server &server, CgiPoll *cgi);
int sendCgiRequest(Server &server, CgiPoll *cgi);

class PollFd {
   protected:
    pollFdHandlerType _readHandler;
    pollFdHandlerType _writeHandler;
    std::shared_ptr<int> _status;
    int _fd;

   public:
    PollFd(int fd);
    virtual ~PollFd();

    std::shared_ptr<int> getStatus();
    int getFd();
    void setWriteHandler(pollFdHandlerType f);
    void setReadHandler(pollFdHandlerType f);
    virtual int handleWrite(Server &server, PollFd *pollFd);
    virtual int handleRead(Server &server, PollFd *pollFd);
};

class ClientPoll : public PollFd {
   private:
    clientPollHandlerType _writeHandler;
    clientPollHandlerType _readHandler;
    HttpRequest *_req;
    HttpResponse *_res;
    time_t _acceptTime;
    std::shared_ptr<int> _cgiPollStatus;

   public:
    ClientPoll(int fd);
    ~ClientPoll();

    time_t getAcceptTime();
    int cgiPollStatus();
    void resetConnection();

    HttpResponse &res();
    HttpRequest &req();

    void setCgiPollStatus(std::shared_ptr<int> cgiStatus);
    void setWriteHandler(clientPollHandlerType f);
    void setReadHandler(clientPollHandlerType f);
    int handleWrite(Server &server, PollFd *pollFd);
    int handleRead(Server &server, PollFd *pollFd);
    int sendInternalError(Server &server);
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
    std::shared_ptr<int> _clientStatus;
    time_t _scriptLaunchTime;

    CgiSockets _cgiSockets;
    int _pid;

   public:
    CgiPoll(CgiSockets &cgiSockets, ClientPoll &client, struct pollfd &structPollfd);
    ~CgiPoll();

    int cgiPid();
    int tryFork();
    void killCgiProcess();
    time_t scriptLaunchTime();
    void closeRequestSockets();

    int clientStatus();
    ClientPoll &client();
    void forkAndexecuteScript(Server &server, const String &cgiResourcePath, const String &cgiScriptCommand);
    void redirectCgiProcessInputOutput();
    void execveScript(const String &cgiScriptResourcePath, const String &cgiScriptCommand);

    void switchToResponseReadableSocket();
    void switchToResponseWritableSocket();
    void switchToRequestReadableSocket();
    void switchToRequestWritableSocket();
    int quitPollSuccess();

    void setReadHandler(CgiPollHandlerType f);
    void setWriteHandler(CgiPollHandlerType f);
    int handleRead(Server &server, PollFd *pollFd);
    int handleWrite(Server &server, PollFd *pollFd);
};

#endif