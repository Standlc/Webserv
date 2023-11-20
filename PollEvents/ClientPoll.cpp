#include "PollFd.hpp"

ClientPoll::ClientPoll(int& fd) : PollFd(fd), _req(fd) {
    _cgiPollStatus = NULL;
    _acceptTime = std::time(0);
}

ClientPoll::~ClientPoll() {
}

time_t ClientPoll::getAcceptTime() {
    return _acceptTime;
}

void ClientPoll::setCgiPollStatus(std::shared_ptr<int> cgiStatus) {
    _cgiPollStatus = cgiStatus;
}

int ClientPoll::cgiPollStatus() {
    if (_cgiPollStatus != NULL)
        return *_cgiPollStatus;
    return 0;
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

int ClientPoll::sendInternalError(Server& server) {
    server.loadDefaultErrPage(500, _res);
    this->setWriteHandler(sendResponseToClient);
    return sendResponseToClient(server, this);
}

////////////////////////////////////////////////////////////////////

int sendResponseToClient(Server& server, ClientPoll* client) {
    int clientSocket = client->getFd();

    // std::cout << "-------------------------\n";
    // std::cout << client->res().headers() << "\n";
    // std::cout << "--------------------------------------------------\n";
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

int checkTimeout(time_t time, int seconds) {
    if (std::time(0) - time > seconds) {
        debug("timing out", "", DIM_RED);
        return 1;
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
    client->res().setCgiResponse(200);
    client->setWriteHandler(sendResponseToClient);
    return sendResponseToClient(server, client);
}

int checkCgiPoll(Server& server, ClientPoll* client) {
    if (client->cgiPollStatus() == -1) {
        return client->sendInternalError(server);
    }
    return 0;
}