#include "PollFd.hpp"

ClientPoll::ClientPoll(int fd, Server& server) : PollFd(fd, server) {
    _req = new HttpRequest(fd);
    _res = new HttpResponse(*_req);
    _cgiPollStatus = (int*)NULL;
    _proxyStatus = (int*)NULL;
    _location = NULL;
}

ClientPoll::ClientPoll(const ClientPoll& other) : PollFd(other._fd, other._server) {
    _location = NULL;
    *this = other;
}

ClientPoll& ClientPoll::operator=(const ClientPoll& other) {
    _writeHandler = other._writeHandler;
    _readHandler = other._readHandler;

    delete _location;
    if (other._location) {
        _location = new LocationBlock(*other._location);
    } else {
        _location = NULL;
    }

    _req = new HttpRequest(other._fd);
    _res = new HttpResponse(*_req);
    _cgiPollStatus = other._cgiPollStatus;
    _proxyStatus = other._proxyStatus;
    return *this;
}

ClientPoll::~ClientPoll() {
    delete _res;
    delete _req;
    delete _location;
}

void ClientPoll::resetConnection() {
    delete _res;
    delete _req;
    _req = new HttpRequest(_fd);
    _res = new HttpResponse(*_req);
    _cgiPollStatus = (int*)NULL;
    _proxyStatus = (int*)NULL;
    delete _location;
    _location = NULL;
    this->resetStartTime();
}

void ClientPoll::setLocation(LocationBlock &loc) {
    delete _location;
    _location = new LocationBlock(loc);
}

void ClientPoll::setCgiPollStatus(SharedPtr& cgiStatus) {
    _cgiPollStatus = cgiStatus;
}

void ClientPoll::setProxyStatus(SharedPtr& proxyStatus) {
    _proxyStatus = proxyStatus;
}

int ClientPoll::proxyPollStatus() {
    return *_proxyStatus;
}

int ClientPoll::cgiPollStatus() {
    return *_cgiPollStatus;
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

void ClientPoll::loadErrorPageFromLocation(LocationBlock* location, int statusCode) {
    try {
        location->loadErrPage(statusCode, *_res, location->getPath());
    } catch (int status) {
        try {
            if (location->serverBlock()) {
                location->serverBlock()->loadErrPage(statusCode, *_res, location->getPath());
            } else {
                throw status;
            }
        } catch (int status) {
            _server.loadDefaultErrPage(status, *_res);
        }
    }
}

int ClientPoll::sendErrorPage(int statusCode) {
    if (statusCode >= -1 && statusCode <= POLLNVAL) {
        statusCode = 500;
    }

    if (_location) {
        this->loadErrorPageFromLocation(_location, statusCode);
    } else {
        _server.loadDefaultErrPage(statusCode, *_res);
    }

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

    debug(">> recieving client request", toString(client->getFd()), CYAN);
    if (req.recvAll(client->getFd()) <= 0) {
        return -1;
    }

    try {
        if (!req.isComplete() && req.resumeParsing() == true) {
            debugParsingSuccesss(req, req.socket(), CYAN);
            client->setReadHandler(NULL);
            client->setWriteHandler(executeClientRequest);
        }
    } catch (int parsingErr) {
        debugParsingErr(req, req.socket(), DIM_RED);
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
        throwIf(!serverBlock, 404);
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