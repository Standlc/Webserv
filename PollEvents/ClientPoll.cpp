#include "PollFd.hpp"

ClientPoll::ClientPoll(int& fd) : PollFd(fd), _req(fd) {
    _cgiPollStatus = NULL;
    _cgiLaunchTime = -1;
    _acceptTime = std::time(0);
    _cgiResponsePipes[0] = 0;
    _cgiResponsePipes[1] = 0;
    _cgiPid = -1;
}

ClientPoll::~ClientPoll() {
    this->killCgiProcess();
}

time_t ClientPoll::getAcceptTime() {
    return _acceptTime;
}

void ClientPoll::setCgiPollStatus(std::shared_ptr<int> cgiStatus) {
    _cgiPollStatus = cgiStatus;
}

time_t ClientPoll::cgiLaunchTime() {
    return _cgiLaunchTime;
}

int ClientPoll::cgiPollStatus() {
    if (_cgiPollStatus != NULL)
        return *_cgiPollStatus;
    return 0;
}

int ClientPoll::cgiPid() {
    return _cgiPid;
}

int* ClientPoll::cgiResponsePipes() {
    return _cgiResponsePipes;
}

HttpResponse& ClientPoll::res() {
    return _res;
}

HttpRequest& ClientPoll::req() {
    return _req;
}

void ClientPoll::setWriteHandler(clientPollHandlerType f) {
    _writeHandler = f;
}

void ClientPoll::setReadHandler(clientPollHandlerType f) {
    _readHandler = f;
}

int ClientPoll::handleWrite(Server& server, PollFd* pollFd) {
    if (_writeHandler == NULL)
        return 0;
    return _writeHandler(server, (ClientPoll*)pollFd);
}

int ClientPoll::handleRead(Server& server, PollFd* pollFd) {
    if (_readHandler == NULL)
        return 0;
    return _readHandler(server, (ClientPoll*)pollFd);
}

void ClientPoll::execveCgi(Server& server, const String& cgiResourcePath, const String& cgiScriptCommand, int cgiReqPipes[2]) {
    _cgiPid = tryPipeAndFork(_cgiResponsePipes);

    if (_cgiPid == 0) {
        try {
            this->redirectCgiProcessInputOutput(_cgiResponsePipes, cgiReqPipes);
            this->executeCgiScript(cgiResourcePath, cgiScriptCommand);
        } catch (int status) {
            _cgiPid = -1;
            exitProgram(server, status);
        }
    }

    _cgiLaunchTime = std::time(0);
}

void ClientPoll::redirectCgiProcessInputOutput(int cgiResPipes[2], int cgiReqPipes[2]) {
    if (dup2(cgiResPipes[1], 1) == -1) {
        debugErr("dup2", strerror(errno));
        throw 1;
    }
    close(cgiResPipes[0]);
    close(cgiResPipes[1]);

    if (cgiReqPipes != NULL) {
        if (dup2(cgiReqPipes[0], 0) == -1) {
            debugErr("dup2", strerror(errno));
            throw 1;
        }
        close(cgiReqPipes[0]);
        close(cgiReqPipes[1]);
    }
}

void ClientPoll::executeCgiScript(const String& cgiScriptResourcePath, const String& cgiScriptCommand) {
    String cgiScriptDir = parseFileDirectory(cgiScriptResourcePath);
    debugErr("cgi dir", &cgiScriptDir[0]);

    if (chdir(&cgiScriptDir[0]) == -1) {
        debugErr("chdir", strerror(errno));
        throw 1;
    }

    String scriptName = "./" + parsePathFileName(cgiScriptResourcePath);
    char* const args[] = {(char*)&cgiScriptCommand[0], (char*)&scriptName[0], NULL};
    debugErr("script name", &scriptName[0]);
    debugErr("command", &cgiScriptCommand[0]);

    if (execve(args[0], args, environ) == -1) {
        debugErr("execve", strerror(errno));
        throw 1;
    }
}

void ClientPoll::killCgiProcess() {
    if (_cgiPid != -1) {
        kill(_cgiPid, SIGKILL);
    }
}

void ClientPoll::closeCgiResponsePipes() {
    close(_cgiResponsePipes[0]);
    close(_cgiResponsePipes[1]);
}

////////////////////////////////////////////////////////////////////

int sendResponseToClient(Server& server, ClientPoll* client) {
    int clientSocket = client->getFd();

    std::cout << "-------------------------\n";
    std::cout << client->res().headers() << "\n";
    std::cout << "--------------------------------------------------\n";
    debug("sending response back to client", std::to_string(clientSocket), GREEN);
    int sendStatus = client->res().sendResponse(clientSocket);
    if (sendStatus == -1) {
        return -1;
    }
    if (sendStatus == 0) {
        // client->setWriteHandler(timeoutClient);
        return -1;
    }
    return 0;
}

int readClientRequest(Server& server, ClientPoll* client) {
    HttpRequest& req = client->req();
    HttpResponse& res = client->res();

    debug("reading client request", std::to_string(client->getFd()), WHITE);
    if (req.recvAll(client->getFd()) == -1) {
        return -1;
    }

    try {
        if (req.isComplete() == false && req.resumeParsing() == true) {
            // std::cout << "--------------------------------------------------\n";
            // std::cout << req.rawData() << "\n";
            // std::cout << "-------------------------\n";
            client->setWriteHandler(executeClientRequest);
        }
    } catch (int statusCode) {
        debug("parsing error", "", RED);
        server.loadDefaultErrPage(statusCode, res);
        client->setWriteHandler(sendResponseToClient);
    }
    return 0;
}

int timeoutClient(Server& server, ClientPoll* client) {
    if (checkTimeout(client->getAcceptTime(), TIMEOUT)) {
        return -1;
    }
    return 0;
}

int executeClientRequest(Server& server, ClientPoll* client) {
    clientPollHandlerType writeHandler = NULL;

    try {
        String socketPort = client->req().getSocketPort();
        ServerBlock& block = server.findServerBlock(socketPort, client->req().getHeader("Host"));
        writeHandler = block.execute(server, *client);
    } catch (int statusCode) {
        server.loadDefaultErrPage(statusCode, client->res());
        writeHandler = sendResponseToClient;
    }

    client->setWriteHandler(writeHandler);
    return writeHandler(server, client);
}

int setCgiResponse(Server& server, ClientPoll* client) {
    HttpResponse& res = client->res();

    if (client->cgiPollStatus() == -1) {
        server.loadDefaultErrPage(500, res);
    } else {
        int* pipes = client->cgiResponsePipes();
        debug("removing cgi pipe", std::to_string(pipes[0]), WHITE);
        int CgiPollIndex = server.findPollFd(pipes[0]);
        server.removePollFd(CgiPollIndex);
        res.setCgiResponse(200);
    }

    client->setWriteHandler(sendResponseToClient);
    return sendResponseToClient(server, client);
}

int waitEmptyCgiPipe(Server& server, ClientPoll* client) {
    debug("waiting for cgi to empty the pipe", std::to_string(client->getFd()), WHITE);
    client->setWriteHandler(setCgiResponse);
    return 0;
}

int handleCgiProcessErr(Server& server, ClientPoll* client) {
    client->closeCgiResponsePipes();
    server.loadDefaultErrPage(500, client->res());
    client->setWriteHandler(sendResponseToClient);
    return sendResponseToClient(server, client);
}

int checkTimeout(time_t time, int seconds) {
    if (std::time(0) - time > seconds) {
        debug("timing out", "", DIM_RED);
        return 1;
    }
    return 0;
}

int waitCgiProcessEnd(Server& server, ClientPoll* client) {
    int cgiPid = client->cgiPid();
    int waitpidStatus;

    // debug("checking cgi pid", std::to_string(cgiPid), WHITE);
    if (waitpid(cgiPid, &waitpidStatus, WNOHANG) == -1) {
        if (WIFEXITED(waitpidStatus) && WEXITSTATUS(waitpidStatus) != 0) {
            debug("cgi process has exit with error", std::to_string(waitpidStatus), RED);
            return handleCgiProcessErr(server, client);
        }

        debug("cgi process has exit", std::to_string(cgiPid), WHITE);
        server.pushNewCgiResponsePoll(client->cgiResponsePipes(), *client);
        client->setWriteHandler(waitEmptyCgiPipe);
    }

    if (checkTimeout(client->cgiLaunchTime(), 15)) {
        client->killCgiProcess();
        return handleCgiProcessErr(server, client);
    }
    return 0;
}

int checkCgiWrite(Server& server, ClientPoll* client) {
    if (client->cgiPollStatus() == -1) {
        debug("cgi status -1", "", WHITE);
        client->killCgiProcess();
        server.loadDefaultErrPage(500, client->res());
        client->setWriteHandler(sendResponseToClient);
        return sendResponseToClient(server, client);
    }

    return waitCgiProcessEnd(server, client);
}