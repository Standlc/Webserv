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

int sendResponseToClient(Server &server, ClientPoll *client);
int executeClientRequest(Server &server, ClientPoll *client);
int readClientRequest(Server &server, ClientPoll *client);
int timeoutClient(Server &server, ClientPoll *client);

int checkCgiWrite(Server &server, ClientPoll *client);
int waitCgiProcessEnd(Server &server, ClientPoll *client);
int readCgiResponseFromPipe(Server &server, CgiPoll *cgi);
int sendCgiRequest(Server &server, CgiPoll *cgi);
int waitEmptyCgiPipe(Server &server, ClientPoll *client);

class PollFd {
   protected:
    pollFdHandlerType _readHandler;
    pollFdHandlerType _writeHandler;
    std::shared_ptr<int> _status;
    int _fd;

   public:
    PollFd(int fd);
    virtual ~PollFd();

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
    HttpRequest _req;
    HttpResponse _res;
    time_t _acceptTime;

    std::shared_ptr<int> _cgiPollStatus;
    int _cgiPid;
    int _cgiResponsePipes[2];
    time_t _cgiLaunchTime;

   public:
    ClientPoll(int &fd);
    ~ClientPoll();

    time_t getAcceptTime();
    void setCgiPollStatus(std::shared_ptr<int> cgiStatus);
    int cgiPollStatus();
    time_t cgiLaunchTime();
    int cgiPid();
    int *cgiResponsePipes();
    std::shared_ptr<int> getStatus();

    HttpResponse &res();
    HttpRequest &req();

    void setWriteHandler(clientPollHandlerType f);
    void setReadHandler(clientPollHandlerType f);
    int handleWrite(Server &server, PollFd *pollFd);
    int handleRead(Server &server, PollFd *pollFd);

    void execveCgi(Server &server, const String &cgiResourcePath, const String &cgiScriptCommand, int cgiReqPipes[2] = NULL);
    void redirectCgiProcessInputOutput(int cgiResPipes[2], int cgiReqPipes[2] = NULL);
    void executeCgiScript(const String &cgiScriptResourcePath, const String &cgiScriptCommand);
    void killCgiProcess();
    void closeCgiResponsePipes();
};

class CgiPoll : public PollFd {
   private:
    CgiPollHandlerType _readHandler;
    CgiPollHandlerType _writeHandler;
    ClientPoll &_client;
    std::shared_ptr<int> _clientStatus;
    int _pipes[2];

   public:
    CgiPoll(int fd, int *pipes, ClientPoll &client);
    ~CgiPoll();

    int clientStatus();
    int *getPipes();
    ClientPoll &client();

    void setReadHandler(CgiPollHandlerType f);
    void setWriteHandler(CgiPollHandlerType f);
    int handleRead(Server &server, PollFd *pollFd);
    int handleWrite(Server &server, PollFd *pollFd);
};

#endif