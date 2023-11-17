#ifndef POLL_FD_HPP
#define POLL_FD_HPP

#include "LocationBlock.hpp"
#include "Server.hpp"
#include "webserv.hpp"

class PollFd;

typedef int (*pollFdHandlerType)(Server &, PollFd *);

class PollFd {
   protected:
    pollFdHandlerType _readHandler;
    pollFdHandlerType _writeHandler;
    int _fd;
    std::shared_ptr<int> _status;
    time_t _acceptTime;

   public:
    virtual ~PollFd() {
        close(_fd);
        *_status = -1;
    };

    PollFd(int fd) : _fd(fd), _status(new int(0)) {
        _acceptTime = std::time(0);
        _readHandler = NULL;
        _writeHandler = NULL;
    }

    time_t getAcceptTime() {
        return _acceptTime;
    }

    std::shared_ptr<int> getStatus() {
        return _status;
    }

    int getFd() {
        return _fd;
    }

    void setWriteHandler(pollFdHandlerType f) {
        _writeHandler = f;
    }

    void setReadHandler(pollFdHandlerType f) {
        _readHandler = f;
    }

    virtual int handleWrite(Server &server, PollFd *pollFd) {
        if (_writeHandler == NULL)
            return 0;
        return _writeHandler(server, pollFd);
    }

    virtual int handleRead(Server &server, PollFd *pollFd) {
        if (_readHandler == NULL)
            return 0;
        return _readHandler(server, pollFd);
    }
};

typedef int (*clientPollFdHandlerType)(Server &, ClientPollFd *);

class ClientPollFd : public PollFd {
   private:
    clientPollFdHandlerType _writeHandler;
    clientPollFdHandlerType _readHandler;
    HttpRequest _req;
    HttpResponse _res;

    int _cgiPid;
    int _cgiResponsePipes[2];
    std::shared_ptr<int> _cgiResponseStatus;
    std::shared_ptr<int> _cgiRequestStatus;
    time_t _cgiLaunchTime;

   public:
    ClientPollFd(int &fd) : PollFd(fd),
                            _cgiResponseStatus(NULL),
                            _cgiRequestStatus(NULL),
                            _cgiLaunchTime(-1) {
        _cgiResponsePipes[0] = 0;
        _cgiResponsePipes[1] = 0;
    }

    void setUpCgi(int pid, int pipes[2]) {
        _cgiPid = pid;
        _cgiLaunchTime = std::time(0);
        _cgiResponsePipes[0] = pipes[0];
        _cgiResponsePipes[1] = pipes[1];
    }

    void setCgiRequestStatus(std::shared_ptr<int> cgiStatus) {
        _cgiRequestStatus = cgiStatus;
    }

    void setCgiResponsStatus(std::shared_ptr<int> cgiStatus) {
        _cgiResponseStatus = cgiStatus;
    }

    time_t cgiLaunchTime() {
        return _cgiLaunchTime;
    }

    int cgiResponseStatus() {
        if (_cgiResponseStatus != NULL)
            return *_cgiResponseStatus;
        return 0;
    }

    int cgiRequestStatus() {
        if (_cgiRequestStatus != NULL)
            return *_cgiRequestStatus;
        return 0;
    }

    int cgiPid() {
        return _cgiPid;
    }

    int *cgiResponsePipes() {
        return _cgiResponsePipes;
    }

    HttpResponse &res() {
        return _res;
    }

    HttpRequest &req() {
        return _req;
    }

    void setWriteHandler(clientPollFdHandlerType f) {
        _writeHandler = f;
    }

    void setReadHandler(clientPollFdHandlerType f) {
        _readHandler = f;
    }

    int handleWrite(Server &server, PollFd *pollFd) {
        if (_writeHandler == NULL)
            return 0;
        return _writeHandler(server, (ClientPollFd *)pollFd);
    }

    int handleRead(Server &server, PollFd *pollFd) {
        if (_readHandler == NULL)
            return 0;
        return _readHandler(server, (ClientPollFd *)pollFd);
    }
};

typedef int (*CgiReadPollFdHandlerType)(Server &, CgiReadPollFd *);

class CgiReadPollFd : public PollFd {
   private:
    CgiReadPollFdHandlerType _readHandler;
    ClientPollFd &_clientPollFd;
    std::shared_ptr<int> _clientStatus;
    int _pid;
    int _pipes[2];

   public:
    CgiReadPollFd(int pid, int pipes[2], ClientPollFd &clientPollFd) : PollFd(pipes[0]),
                                                                       _clientPollFd(clientPollFd),
                                                                       _clientStatus(clientPollFd.getStatus()),
                                                                       _pid(pid) {
        _pipes[0] = pipes[0];
        _pipes[1] = pipes[1];
    }

    ~CgiReadPollFd() {
        close(_pipes[0]);
        close(_pipes[1]);
    }

    int getClientStatus() {
        if (_clientStatus != NULL)
            return *_clientStatus;
        return 0;
    }

    int getPid() {
        return _pid;
    }

    int *getPipes() {
        return _pipes;
    }

    ClientPollFd &client() {
        return _clientPollFd;
    }

    void setReadHandler(CgiReadPollFdHandlerType f) {
        _readHandler = f;
    }

    int handleRead(Server &server, PollFd *pollFd) {
        if (_readHandler == NULL)
            return 0;
        return _readHandler(server, (CgiReadPollFd *)pollFd);
    }
};

class LocationBlock;
typedef int (*CgiWritePollFdHandlerType)(Server &, CgiWritePollFd *);

class CgiWritePollFd : public PollFd {
   private:
    CgiWritePollFdHandlerType _writeHandler;
    ClientPollFd &_clientPollFd;
    std::shared_ptr<int> _clientStatus;
    int _pipes[2];
    int _cgiResPipes[2];

    LocationBlock &_reqLocation;
    std::string _cgiFile;

   public:
    CgiWritePollFd(int pipes[2], ClientPollFd &clientPollFd, LocationBlock &location, std::string cgiFile) : PollFd(pipes[1]),
                                                                                                             _clientPollFd(clientPollFd),
                                                                                                             _clientStatus(clientPollFd.getStatus()),
                                                                                                             _reqLocation(location),
                                                                                                             _cgiFile(cgiFile) {
        _pipes[0] = pipes[0];
        _pipes[1] = pipes[1];
    }

    ~CgiWritePollFd() {
        close(_pipes[0]);
        close(_pipes[1]);
    }

    int *createPipe() {
        if (pipe(_cgiResPipes) == -1)
            return NULL;
        return _cgiResPipes;
    }

    std::string getCgiFile() {
        return _cgiFile;
    }

    int clientStatus() {
        if (_clientStatus != NULL)
            return *_clientStatus;
        return 0;
    }

    int *getPipes() {
        return _pipes;
    }

    LocationBlock &getLocation() {
        return _reqLocation;
    }

    ClientPollFd &client() {
        return _clientPollFd;
    }

    void setWriteHandler(CgiWritePollFdHandlerType f) {
        _writeHandler = f;
    }

    int handleWrite(Server &server, PollFd *pollFd) {
        if (_writeHandler == NULL)
            return 0;
        return _writeHandler(server, (CgiWritePollFd *)pollFd);
    }
};

#endif