#include "PollFd.hpp"

CgiPoll::CgiPoll(CgiSockets &cgiSockets, ClientPoll &client, struct pollfd &structPollfd) : PollFd(structPollfd.fd, client.server()),
                                                                                            _pollfd(structPollfd),
                                                                                            _client(client) {
    _cgiSockets = cgiSockets;
    _pid = -1;
    _clientStatus = client.getStatus();
    client.setCgiPollStatus(_status);
    _cgiReq = new CgiRequest(client.req());
    _cgiRes = new CgiResponse(client.res());
}

CgiPoll::~CgiPoll() {
    closeOpenFd(_cgiSockets.response[0]);
    closeOpenFd(_cgiSockets.response[1]);
    closeOpenFd(_cgiSockets.request[0]);
    closeOpenFd(_cgiSockets.request[1]);
    delete _cgiReq;
    delete _cgiRes;
    this->killCgiProcess();
}

void CgiPoll::killCgiProcess() {
    if (_pid != -1) {
        kill(_pid, SIGKILL);
        _pid = -1;
    }
}

int CgiPoll::clientStatus() {
    return *_clientStatus;
}

void CgiPoll::closeRequestSockets() {
    closeOpenFd(_cgiSockets.request[0]);
    closeOpenFd(_cgiSockets.request[1]);
}

ClientPoll &CgiPoll::client() {
    return _client;
}

void CgiPoll::setReadHandler(CgiPollHandlerType f) {
    _readHandler = f;
}

int CgiPoll::handleRead(PollFd *pollFd) {
    if (_readHandler == NULL) {
        return 0;
    }
    return _readHandler((CgiPoll *)pollFd);
}

void CgiPoll::setWriteHandler(CgiPollHandlerType f) {
    _writeHandler = f;
}

int CgiPoll::handleWrite(PollFd *pollFd) {
    if (_writeHandler == NULL) {
        return 0;
    }
    return _writeHandler((CgiPoll *)pollFd);
}

void CgiPoll::switchToResponseReadableSocket() {
    _fd = _cgiSockets.response[0];
    _pollfd.fd = _fd;
    _pollfd.revents = 0;
}

void CgiPoll::switchToRequestWritableSocket() {
    _fd = _cgiSockets.request[1];
    _pollfd.fd = _fd;
    _pollfd.revents = 0;
}

int CgiPoll::tryFork() {
    int pid = fork();
    if (pid == -1) {
        debugErr("fork", strerror(errno));
        this->setWriteHandler(cgiQuitPoll);
        throw 500;
    }
    return pid;
}

void execveScript(Server *server, String cgiScriptResourcePath, String cgiScriptCommand) {
    server->deleteResource();

    String cgiScriptDir = parseFileDirectory(cgiScriptResourcePath);
    String scriptName = "./" + parsePathFileName(cgiScriptResourcePath);

    if (chdir(&cgiScriptDir[0]) == -1) {
        debugErr("chdir", strerror(errno));
        throw 1;
    }

    char *args[] = {(char *)&cgiScriptCommand[0], (char *)&scriptName[0], NULL};
    if (execve(args[0], &args[0], environ) == -1) {
        debugErr("execve", strerror(errno));
        throw 1;
    }
}

void preventChildFromKillingItself(int &pid) {
    pid = -1;
}

void CgiPoll::forkAndexecuteScript(Server &server, const String &cgiResourcePath, const String &cgiScriptCommand) {
    debug("> forking and executing CGI script, socket", toString(_fd), PURPLE);
    _pid = tryFork();

    if (_pid == 0) {
        try {
            preventChildFromKillingItself(_pid);
            this->redirectCgiProcessInputOutput();
            execveScript(&server, cgiResourcePath, cgiScriptCommand);
        } catch (int status) {
            // std::exit(1);
            throw std::runtime_error("CGI Program Failed.");
        }
    }
    this->resetStartTime();
}

void CgiPoll::redirectCgiProcessInputOutput() {
    if (dup2(_cgiSockets.request[0], 0) == -1) {
        debugErr("dup2", strerror(errno));
        throw 1;
    }
    if (dup2(_cgiSockets.response[1], 1) == -1) {
        debugErr("dup2", strerror(errno));
        throw 1;
    }
    closeOpenFd(_cgiSockets.response[0]);
    closeOpenFd(_cgiSockets.response[1]);
    closeOpenFd(_cgiSockets.request[0]);
    closeOpenFd(_cgiSockets.request[1]);
}

CgiRequest &CgiPoll::cgiReq() {
    return *_cgiReq;
}

CgiResponse &CgiPoll::cgiRes() {
    return *_cgiRes;
}

int CgiPoll::cgiPid() {
    return _pid;
}

int cgiQuitPoll(CgiPoll *cgi) {
    int pid = cgi->cgiPid();

    if (pid != -1) {
        int waitPidStatus = waitpid(pid, NULL, WNOHANG);
        if (waitPidStatus == pid || waitPidStatus == -1) {
            return *(cgi->getStatus());
        }
        return 0;
    }
    return pid == -1 ? *(cgi->getStatus()) : 0;
}

int handleCgiQuit(CgiPoll *cgi, int status) {
    cgi->setStatus(status);
    cgi->setReadHandler(NULL);
    cgi->setWriteHandler(cgiQuitPoll);
    return cgiQuitPoll(cgi);
}

int readCgiResponse(CgiPoll *cgi) {
    if (cgi->clientStatus() != 0) {
        return handleCgiQuit(cgi, -1);
    }

    debug(">> reading CGI response", toString(cgi->getFd()), PURPLE);
    CgiResponse &cgiRes = cgi->cgiRes();
    if (cgiRes.recvAll(cgi->getFd()) <= 0) {
        return handleCgiQuit(cgi, 500);
    }

    try {
        if (!cgiRes.isComplete() && cgiRes.resumeParsing()) {
            return handleCgiResponse(cgi);
        }
        cgi->setWriteHandler(waitCgiProcessEnd);
    } catch (int parsingErr) {
        debugParsingErr(cgiRes, cgi->getFd(), DIM_RED);
        return handleCgiQuit(cgi, 502);
    }
    return 0;
}

int handleCgiResponse(CgiPoll *cgi) {
    CgiResponse &cgiRes = cgi->cgiRes();
    if (cgi->clientStatus() != 0) {
        return handleCgiQuit(cgi, 502);
    }
    debugParsingSuccesss(cgiRes, cgi->getFd(), PURPLE);

    try {
        cgi->cgiRes().parseStatusHeader();
        const String &locationRedirect = cgiRes.getHeader("Location");

        if (locationRedirect != "" && !startsWith(locationRedirect, "http://") && !startsWith(locationRedirect, "https://")) {
            debug("> internal redirect >>", "", PURPLE);
            cgi->client().req().setUrl(locationRedirect);
            cgi->client().setWriteHandler(executeClientRequest);
        } else {
            debug("> setting client response, socket", toString(cgi->client().getFd()), PURPLE);
            cgiRes.setClientResponse();
            cgi->client().setWriteHandler(sendResponseToClient);
        }
    } catch (int status) {
        return handleCgiQuit(cgi, 502);
    }
    return handleCgiQuit(cgi, -1);
}

int waitCgiProcessEnd(CgiPoll *cgi) {
    if (cgi->cgiPid() == -1) {
        cgi->setWriteHandler(handleCgiResponse);
        return 0;
    }

    if (cgi->clientStatus() != 0 || checkTimeout(cgi->startTime(), CGI_TIMEOUT)) {
        return handleCgiQuit(cgi, 504);
    }

    int pid = cgi->cgiPid();
    int exitStatus = 0;
    int waitPidStatus = waitpid(pid, &exitStatus, WNOHANG);
    if (waitPidStatus == pid || waitPidStatus == -1) {
        if (WIFEXITED(exitStatus) && WEXITSTATUS(exitStatus) != 0) {
            debug("CGI process has exit with an error", toString(pid), GRAY);
            return handleCgiQuit(cgi, 502);
        }

        debug("CGI process has exit", toString(pid), GRAY);
        cgi->setCgiPid(-1);
        cgi->setWriteHandler(handleCgiResponse);
    }
    return 0;
}

int sendCgiRequest(CgiPoll *cgi) {
    if (cgi->clientStatus() != 0 || checkTimeout(cgi->startTime(), CGI_TIMEOUT)) {
        return handleCgiQuit(cgi, 504);
    }

    int sendStatus = cgi->cgiReq().send(cgi->getFd());
    if (sendStatus == 0) {
        cgi->closeRequestSockets();
        cgi->switchToResponseReadableSocket();
        cgi->setReadHandler(readCgiResponse);
        cgi->setWriteHandler(waitCgiProcessEnd);
    }
    if (sendStatus == -1) {
        return handleCgiQuit(cgi, 500);
    }
    return 0;
}
