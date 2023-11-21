#include "PollFd.hpp"

CgiPoll::CgiPoll(CgiSockets &cgiSockets, ClientPoll &client, struct pollfd &structPollfd) : PollFd(structPollfd.fd),
                                                                                            _pollfd(structPollfd),
                                                                                            _client(client) {
    _cgiSockets = cgiSockets;
    _scriptLaunchTime = -1;
    _pid = -1;
    _clientStatus = client.getStatus();
    client.setCgiPollStatus(_status);
}

CgiPoll::~CgiPoll() {
    closeOpenFd(_cgiSockets.response[0]);
    closeOpenFd(_cgiSockets.response[1]);
    closeOpenFd(_cgiSockets.request[0]);
    closeOpenFd(_cgiSockets.request[1]);
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

int CgiPoll::handleRead(Server &server, PollFd *pollFd) {
    if (_readHandler == NULL) {
        return 0;
    }
    return _readHandler(server, (CgiPoll *)pollFd);
}

void CgiPoll::setWriteHandler(CgiPollHandlerType f) {
    _writeHandler = f;
}

int CgiPoll::handleWrite(Server &server, PollFd *pollFd) {
    if (_writeHandler == NULL) {
        return 0;
    }
    return _writeHandler(server, (CgiPoll *)pollFd);
}

void CgiPoll::switchToResponseReadableSocket() {
    _fd = _cgiSockets.response[0];
    _pollfd.fd = _fd;
    _pollfd.revents = 0;
    this->setReadHandler(readCgiResponse);
    this->setWriteHandler(waitCgiProcessEnd);
}
void CgiPoll::switchToResponseWritableSocket() {
    _fd = _cgiSockets.response[1];
    _pollfd.fd = _fd;
    _pollfd.revents = 0;
}
void CgiPoll::switchToRequestReadableSocket() {
    _fd = _cgiSockets.request[0];
    _pollfd.fd = _fd;
    _pollfd.revents = 0;
}
void CgiPoll::switchToRequestWritableSocket() {
    _fd = _cgiSockets.request[1];
    _pollfd.fd = _fd;
    _pollfd.revents = 0;
    this->setReadHandler(NULL);
    this->setWriteHandler(sendCgiRequest);
}

int CgiPoll::quitPollSuccess() {
    _client.res().setCgiResponse(200);
    _client.setWriteHandler(sendResponseToClient);
    return -1;
}

int CgiPoll::tryFork() {
    int pid = fork();
    if (pid == -1) {
        debugErr("fork", strerror(errno));
        this->setWriteHandler(quitPollError);
        throw 500;
    }
    return pid;
}

void CgiPoll::forkAndexecuteScript(Server &server, const String &cgiResourcePath, const String &cgiScriptCommand) {
    _pid = tryFork();

    if (_pid == 0) {
        try {
            this->redirectCgiProcessInputOutput();
            this->execveScript(cgiResourcePath, cgiScriptCommand);
        } catch (int status) {
            _pid = -1;
            exitProgram(&server, status);
        }
    }

    _scriptLaunchTime = std::time(0);
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

    std::vector<char *> args(1, NULL);
    if (cgiScriptCommand != "") {
        args.insert(args.begin() + 0, (char *)&cgiScriptCommand[0]);
        args.insert(args.begin() + 1, (char *)&scriptName[0]);
    } else {
        args.insert(args.begin() + 0, (char *)&scriptName[0]);
    }

    if (execve(args[0], &args[0], environ) == -1) {
        debugErr("execve", strerror(errno));
        throw 1;
    }
}

time_t CgiPoll::scriptLaunchTime() {
    return _scriptLaunchTime;
}

int CgiPoll::cgiPid() {
    return _pid;
}

//////////////////////////////////////////////////////////////////

int quitPollError(Server &server, CgiPoll *cgi) {
    debug("----------6----------", "", RED);
    return -1;
}

int readCgiResponse(Server &server, CgiPoll *cgi) {
    if (cgi->clientStatus() == -1) {
        debug("----------3----------", "", RED);
        return -1;
    }

    ClientPoll &client = cgi->client();
    debug("reading cgi response from socket", std::to_string(cgi->getFd()), WHITE);
    if (client.res().recvAll(cgi->getFd()) == -1) {
        debug("----------2----------", "", RED);
        return -1;
    }
    return 0;
}

int waitCgiProcessEnd(Server &server, CgiPoll *cgi) {
    if (cgi->clientStatus() == -1 || checkTimeout(cgi->scriptLaunchTime(), CGI_TIMEOUT)) {
        debug("----------1----------", "", RED);
        return -1;
    }

    int pid = cgi->cgiPid();
    int exitStatus = 0;

    if (waitpid(pid, &exitStatus, WNOHANG) == -1) {
        if (WIFEXITED(exitStatus) && WEXITSTATUS(exitStatus) != 0) {
            debug("cgi process has exit with an error", std::to_string(WEXITSTATUS(exitStatus)), RED);
            return -1;
        }
        debug("cgi process has exit", std::to_string(pid), WHITE);
        return cgi->quitPollSuccess();
    }
    return 0;
}

int sendCgiRequest(Server &server, CgiPoll *cgi) {
    if (cgi->clientStatus() == -1) {
        return -1;
    }

    debug("sending cgi request", std::to_string(cgi->getFd()), WHITE);
    int sendStatus = cgi->client().req().sendBody(cgi->getFd());
    if (sendStatus == 0) {
        cgi->closeRequestSockets();
        cgi->switchToResponseReadableSocket();
    }
    return (sendStatus == -1) ? -1 : 0;
}
