#include "Server.hpp"

Server::Server() : _serverBlockSize(0) {
}

Server::~Server() {
    this->deleteResource();
}

void Server::deleteResource() {
    for (size_t i = 0; i < _pollFds.size(); i++) {
        delete _pollFds[i];
    }
    _pollFds.resize(0);
}

struct addrinfo* getServerAddressInfo(String serverIpAddress, String port) {
    struct addrinfo* res;
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(&serverIpAddress[0], &port[0], &hints, &res) != 0) {
        debugErr("getaddrinfo", gai_strerror(errno));
        throw errno;
    }
    return res;
}

int createSocket(struct addrinfo* addrInfo) {
    int socketFd = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
    if (socketFd == -1) {
        debugErr("socket", strerror(errno));
        freeaddrinfo(addrInfo);
        throw errno;
    }
    return socketFd;
}

void bindSocket(int socketFd, struct addrinfo* addrInfo) {
    int yes = 1;
    if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        debugErr("setsockopt", strerror(errno));
        closeOpenFd(socketFd);
        freeaddrinfo(addrInfo);
        throw errno;
    }

    if (bind(socketFd, addrInfo->ai_addr, addrInfo->ai_addrlen) == -1) {
        debugErr("bind", strerror(errno));
        closeOpenFd(socketFd);
        freeaddrinfo(addrInfo);
        throw errno;
    }
}

void printServerHostnames(ServerBlock& serverBlock) {
    if (serverBlock.hostNames().size()) {
        String hostnames;
        for (size_t i = 0; i < serverBlock.hostNames().size(); i++) {
            hostnames += serverBlock.hostNames()[i];
            if (i < serverBlock.hostNames().size() - 1) {
                hostnames += ", ";
            }
        }
        std::cout << " (" << hostnames << ")";
    }
}

void printServerListeningMessage(ServerBlock& serverBlock) {
    String host = serverBlock.ipAddress();
    std::cout << "\033[0;92m> " << host;

    printServerHostnames(serverBlock);

    std::cout << " is listening on port " << serverBlock.port() << "..."
              << WHITE << "\n";
}

void printServerStartFail(ServerBlock& serverBlock) {
    std::cerr << RED << " ↳ " << serverBlock.ipAddress();
    printServerHostnames(serverBlock);
    std::cout << " could not be started on port " << serverBlock.port() << WHITE << "\n";
}

void listenToSocket(int socketFd, ServerBlock& serverBlock) {
    if (listen(socketFd, 64) == -1) {
        debugErr("listen", strerror(errno));
        closeOpenFd(socketFd);
        throw errno;
    }
    printServerListeningMessage(serverBlock);
}

int Server::listen() {
    if (this->startServers() == 0) {
        return errno;
    }
    return this->monitorClients();
}

int Server::startServers() {
    std::vector<String> usedPorts;
    struct addrinfo* serverInfo;

    for (int i = 0; i < _serverBlockSize; i++) {
        try {
            if (std::find(usedPorts.begin(), usedPorts.end(), _blocks[i].port()) != usedPorts.end()) {
                printServerListeningMessage(_blocks[i]);
                continue;
            }

            serverInfo = getServerAddressInfo(_blocks[i].ipAddress(), _blocks[i].port());
            int socket = createSocket(serverInfo);
            bindSocket(socket, serverInfo);
            freeaddrinfo(serverInfo);
            listenToSocket(socket, _blocks[i]);

            this->pushNewListeningSocket(socket);
            usedPorts.push_back(_blocks[i].port());
        } catch (int err) {
            printServerStartFail(_blocks[i]);
        }
    }
    return usedPorts.size();
}

int Server::monitorClients() {
    while (true) {
        int nEvents = poll(&_fds[0], _fds.size(), -1);
        if (nEvents == -1) {
            debugErr("poll", strerror(errno));
            return errno;
        }
        this->scanForEventSockets(nEvents);
    }
    return 0;
}

void Server::scanForEventSockets(int nEvents) {
    int eventsFound = 0;
    _fdsSize = _fds.size();

    for (int i = 0; i < _fdsSize && eventsFound < nEvents; i++) {
        int status = checkPollErrors(_fds[i]);
        if (status != 0 && this->handleSocketErrors(&i, status)) {
            continue;
        }

        eventsFound += (isReadable(_fds[i]) || isWritable(_fds[i]));
        if (_pollFds[i]->isConcurrentReadWrite()) {
            status = this->checkReadableANDWritable(i);
        } else {
            status = this->checkReadableORWritable(i);
        }

        if (status != 0) {
            this->handleSocketErrors(&i, status);
        }
    }
}

int Server::handleSocketErrors(int* i, int socketStatus) {
    if (socketStatus == POLLHUP && _pollFds[*i]->removeOnHungUp() == false) {
        return 0;
    }
    this->removePollFd(*i, socketStatus);
    *i -= 1;
    return socketStatus;
}

int Server::checkReadableANDWritable(int i) {
    int returnStatus = 0;

    if (isReadable(_fds[i])) {
        returnStatus = _pollFds[i]->handleRead(_pollFds[i]);
    }
    if (!returnStatus && isWritable(_fds[i])) {
        returnStatus = _pollFds[i]->handleWrite(_pollFds[i]);
    }
    return returnStatus;
}

int Server::checkReadableORWritable(int i) {
    int returnStatus = 0;

    if (isReadable(_fds[i])) {
        returnStatus = _pollFds[i]->handleRead(_pollFds[i]);
    } else if (!returnStatus && isWritable(_fds[i])) {
        returnStatus = _pollFds[i]->handleWrite(_pollFds[i]);
    }
    return returnStatus;
}

void Server::removePollFd(int index, int socketStatus) {
    debug("<= closing connection", toString(_fds[index].fd), GRAY);
    _pollFds[index]->destroy(socketStatus);
    delete _pollFds[index];
    _pollFds.erase(_pollFds.begin() + index);
    _fds.erase(_fds.begin() + index);
    _fdsSize--;
}

void Server::pushNewProxy(int proxySocket, ClientPoll& client, ProxyUrl& proxyPass) {
    this->pushStructPollfd(proxySocket);

    ProxyPoll* newProxy = new ProxyPoll(proxySocket, client, proxyPass);
    newProxy->setWriteHandler(sendProxyRequest);
    newProxy->setReadHandler(NULL);
    _pollFds.push_back(newProxy);
}

void Server::pushNewClient(int clientSocket) {
    this->pushStructPollfd(clientSocket);

    ClientPoll* newClient = new ClientPoll(clientSocket, *this);
    newClient->setReadHandler(readClientRequest);
    newClient->setWriteHandler(timeoutClient);
    _pollFds.push_back(newClient);
}

void Server::pushNewListeningSocket(int listeningSocket) {
    this->pushStructPollfd(listeningSocket);

    PollFd* newListeningPollFd = new PollFd(listeningSocket, *this);
    newListeningPollFd->setReadHandler(handleNewConnection);
    _pollFds.push_back(newListeningPollFd);
}

CgiPoll& Server::pushNewCgiPoll(CgiSockets& cgiSockets, ClientPoll& client) {
    this->pushStructPollfd(cgiSockets.response[0]);
    fcntl(cgiSockets.request[1], F_SETFL, O_NONBLOCK, FD_CLOEXEC);

    CgiPoll* newCgi = new CgiPoll(cgiSockets, client, _fds[_fds.size() - 1]);
    newCgi->setWriteHandler(waitCgiProcessEnd);
    newCgi->setReadHandler(readCgiResponse);
    _pollFds.push_back(newCgi);
    return *newCgi;
}

void Server::pushStructPollfd(int fd) {
    fcntl(fd, F_SETFL, O_NONBLOCK, FD_CLOEXEC);
    struct pollfd element;
    element.fd = fd;
    element.events = POLLIN | POLLOUT;
    element.revents = 0;
    _fds.push_back(element);
}

void Server::loadDefaultErrPage(int statusCode, HttpResponse& res) {
    try {
        res.loadFile(statusCode, this->getDefaultErrorPagePath(statusCode));
    } catch (int status) {
        if (status == 500) {
            res.set(500, ".txt", "The server encountered some issue while handling your request");
        } else {
            res.set(statusCode, ".txt", toString(statusCode) + " Error");
        }
    }
}

String Server::getDefaultErrorPagePath(int statusCode) {
    return "defaultPages/error/" + toString(statusCode) + ".html";
}

ServerBlock* Server::findServerBlock(HttpRequest& req) {
    ServerBlock* defaultBlock = NULL;
    String port = req.getSocketPort();
    String host = req.getHeader("Host");
    String hostName = host.substr(0, host.find_first_of(':'));
    if (hostName == "") {
        hostName = req.getSocketIpAddress();
    }

    for (int i = 0; i < _serverBlockSize; i++) {
        if (_blocks[i].port() == port) {
            if (_blocks[i].isHost(hostName)) {
                return &_blocks[i];
            }
            if (!defaultBlock) {
                defaultBlock = &_blocks[i];
            }
        }
    }
    return defaultBlock;
}

ServerBlock* Server::addBlock() {
    _serverBlockSize++;

    ServerBlock block;
    _blocks.push_back(block);
    return &_blocks[_blocks.size() - 1];
}

ServerBlock& Server::getServerBlock(int index) {
    return _blocks[index];
}

int Server::getServerSize() {
    return _blocks.size();
}

int handleNewConnection(PollFd* listen) {
    int newClientSocket = accept(listen->getFd(), NULL, NULL);
    if (newClientSocket == -1) {
        debugErr("accept", strerror(errno));
        return 0;
    }
    debug("=> adding new client", toString(newClientSocket), GREEN);
    listen->server().pushNewClient(newClientSocket);
    return 0;
}
