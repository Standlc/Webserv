#include "PollFd.hpp"

CgiPoll::CgiPoll(CgiSockets &cgiSockets, ClientPoll &client, struct pollfd &structPollfd) : PollFd(structPollfd.fd, client.server()),
                                                                                            _pollfd(structPollfd),
                                                                                            _client(client) {
    _cgiSockets = cgiSockets;
    _pid = -1;
    _clientStatus = client.getStatus();
    client.setCgiPollStatus(_status);
    // _concurrentReadWrite = false;
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
    if (_clientStatus != NULL) {
        return *_clientStatus;
    }
    return 0;
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

void CgiPoll::forkAndexecuteScript(const String &cgiResourcePath, const String &cgiScriptCommand) {
    debug("> forking and executing CGI script, socket", std::to_string(_fd), PURPLE);
    _pid = tryFork();

    if (_pid == 0) {
        try {
            this->redirectCgiProcessInputOutput();
            this->execveScript(cgiResourcePath, cgiScriptCommand);
        } catch (int status) {
            _pid = -1;
            handleSigint(1);
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

void CgiPoll::execveScript(const String &cgiScriptResourcePath, const String &cgiScriptCommand) {
    String cgiScriptDir = parseFileDirectory(cgiScriptResourcePath);
    String scriptName = "./" + parsePathFileName(cgiScriptResourcePath);
    // debugErr("cgi dir", &cgiScriptDir[0]);
    // debugErr("script name", &scriptName[0]);
    // debugErr("command", &cgiScriptCommand[0]);

    if (chdir(&cgiScriptDir[0]) == -1) {
        debugErr("chdir", strerror(errno));
        throw 1;
    }

    char *args[] = {(char *)&cgiScriptCommand[0], (char *)&scriptName[0], NULL};
    // char *args[] = {(char *)"/opt/homebrew/bin/php-cgi", (char *)"./php.php", NULL};
    // debug("arg 0", String(args[0]));
    // debug("arg 1", String(args[1]));

    if (execve(args[0], &args[0], environ) == -1) {
        debugErr("execve", strerror(errno));
        throw 1;
    }
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
    if (cgi->cgiPid() != -1 && waitpid(cgi->cgiPid(), NULL, WNOHANG) == -1) {
        return -1;
    }
    return 0;
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

    debug(">> reading CGI response", std::to_string(cgi->getFd()), PURPLE);
    CgiResponse &cgiRes = cgi->cgiRes();
    if (cgiRes.recvAll(cgi->getFd()) <= 0) {
        return handleCgiQuit(cgi, 500);
    }

    try {
        if (!cgiRes.isComplete() && cgiRes.resumeParsing()) {
            debugParsingSuccesss(cgiRes, cgi->getFd(), PURPLE);
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
    if (cgi->clientStatus() != 0 || !cgiRes.isComplete()) {
        return handleCgiQuit(cgi, 502);
    }

    const String &locationRedirect = cgiRes.getHeader("Location");
    if (locationRedirect != "" && locationRedirect[0] == '/') {
        debug("> internal redirect >>", "", PURPLE);
        cgi->client().req().setUrl(locationRedirect);
        cgi->client().setWriteHandler(executeClientRequest);
    } else {
        debug("> setting client response, socket", std::to_string(cgi->client().getFd()), PURPLE);
        cgiRes.setClientResponse();
        cgi->client().setWriteHandler(sendResponseToClient);
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
    if (waitpid(pid, &exitStatus, WNOHANG) == -1) {
        if (WIFEXITED(exitStatus) && WEXITSTATUS(exitStatus) != 0) {
            debug("CGI process has exit with an error", std::to_string(WEXITSTATUS(exitStatus)), RED);
            return handleCgiQuit(cgi, 502);
        }

        debug("CGI process has exit", std::to_string(pid), GRAY);
        cgi->setCgiPid(-1);
        cgi->setWriteHandler(handleCgiResponse);
    }
    return 0;
}

int sendCgiRequest(CgiPoll *cgi) {
    if (cgi->clientStatus() != 0) {
        return handleCgiQuit(cgi, -1);
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
