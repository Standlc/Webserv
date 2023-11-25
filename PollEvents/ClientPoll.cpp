#include "PollFd.hpp"

ClientPoll::ClientPoll(int fd, Server& server) : PollFd(fd, server) {
    _req = new HttpRequest(fd);
    _res = new HttpResponse(*_req);
    _cgiPollStatus = NULL;
    _proxyStatus = NULL;
}

ClientPoll::~ClientPoll() {
    delete _res;
    delete _req;
}

void ClientPoll::resetConnection() {
    delete _res;
    delete _req;
    _req = new HttpRequest(_fd);
    _res = new HttpResponse(*_req);
    _cgiPollStatus = NULL;
    _proxyStatus = NULL;
    this->resetStartTime();
}

void ClientPoll::setCgiPollStatus(std::shared_ptr<int> cgiStatus) {
    _cgiPollStatus = cgiStatus;
}

void ClientPoll::setProxyStatus(std::shared_ptr<int> proxyStatus) {
    _proxyStatus = proxyStatus;
}

int ClientPoll::proxyPollStatus() {
    if (_proxyStatus != NULL) {
        return *_proxyStatus;
    }
    return *_proxyStatus;
}

int ClientPoll::cgiPollStatus() {
    if (_cgiPollStatus != NULL) {
        return *_cgiPollStatus;
    }
    return 0;
}

HttpResponse& ClientPoll::res() {
    return *_res;
}

HttpRequest& ClientPoll::req() {
    return *_req;
}

void ClientPoll::setWriteHandler(clientPollHandlerType f) {
    _writeHandler = f;
}

void ClientPoll::setReadHandler(clientPollHandlerType f) {
    _readHandler = f;
}

int ClientPoll::handleWrite(PollFd* pollFd) {
    if (_writeHandler == NULL)
        return 0;
    return _writeHandler((ClientPoll*)pollFd);
}

int ClientPoll::handleRead(PollFd* pollFd) {
    if (_readHandler == NULL)
        return 0;
    return _readHandler((ClientPoll*)pollFd);
}

int ClientPoll::sendErrorPage(int statusCode) {
    if (statusCode >= -1 && statusCode <= POLLNVAL) {
        statusCode = 500;
    }

    // try {
    //     _location->loadErrPage(statusCode, *_res, *_req);
    // } catch (int status) {
    //     try {
    //         _location->serverBlock().loadErrPage(statusCode, *_res, *_req);
    //     } catch (int status) {
    //         _server.loadDefaultErrPage(status, *_res);
    //     }
    // }
    _server.loadDefaultErrPage(statusCode, *_res);
    this->setWriteHandler(sendResponseToClient);
    return sendResponseToClient(this);
}

int sendResponseToClient(ClientPoll* client) {
    int clientSocket = client->getFd();

    int sendStatus = client->res().sendResponse(clientSocket);
    if (sendStatus == 0) {
        if (client->res().keepAlive() == false) {
            return -1;
        }
        client->setWriteHandler(timeoutClient);
        client->setReadHandler(readClientRequest);
        client->resetConnection();
    }
    return (sendStatus == -1) ? -1 : 0;
}

int readClientRequest(ClientPoll* client) {
    HttpRequest& req = client->req();

    debug(">> recieving client request", std::to_string(client->getFd()), CYAN);
    if (req.recvAll(client->getFd()) <= 0) {
        return -1;
    }

    try {
        if (!req.isComplete() && req.resumeParsing() == true) {
            debugHttpMessage(req.rawData(), CYAN);
            client->setReadHandler(NULL);
            client->setWriteHandler(executeClientRequest);
        }
    } catch (int parsingErr) {
        client->setReadHandler(NULL);
        client->server().loadDefaultErrPage(400, client->res());
        client->setWriteHandler(sendResponseToClient);
    }
    return 0;
}

int timeoutClient(ClientPoll* client) {
    if (checkTimeout(client->startTime(), TIMEOUT)) {
        return -1;
    }
    return 0;
}

int checkTimeout(time_t time, int seconds) {
    if (std::time(0) - time > seconds) {
        debug("connection timed out", "", DIM_RED);
        return 1;
    }
    return 0;
}

int executeClientRequest(ClientPoll* client) {
    clientPollHandlerType writeHandler = NULL;

    try {
        ServerBlock* serverBlock = client->server().findServerBlock(client->req());
        throwIf(!serverBlock, 400);
        writeHandler = serverBlock->execute(*client);
    } catch (int statusCode) {
        return client->sendErrorPage(statusCode);
    }

    client->setWriteHandler(writeHandler);
    return writeHandler(client);
}

int checkCgiPoll(ClientPoll* client) {
    int cgiStatus = client->cgiPollStatus();
    if (cgiStatus != 0) {
        return client->sendErrorPage(cgiStatus);
    }
    return 0;
}

int checkProxyPoll(ClientPoll* client) {
    int proxyStatus = client->proxyPollStatus();
    if (proxyStatus != 0) {
        return client->sendErrorPage(proxyStatus);
    }
    return 0;
}