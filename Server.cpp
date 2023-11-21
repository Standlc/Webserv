#include "Server.hpp"

Server::Server() : _serverBlockSize(0) {
}

Server::~Server() {
    for (size_t i = 0; i < _pollFds.size(); i++) {
        delete _pollFds[i];
    }
}

struct addrinfo* getServerAddressInfo(String serverIpAddress, String port) {
    struct addrinfo* res;
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    char* ipAddress = (serverIpAddress != "") ? &serverIpAddress[0] : NULL;
    int status = getaddrinfo(ipAddress, &port[0], &hints, &res);
    if (status == -1) {
        debugErr("getaddrinfo", gai_strerror(errno));
        throw -1;
    }
    if (res == NULL) {
        debugErr("getaddrinfo", gai_strerror(errno));
        throw -1;
    }
    return res;
}

int createSocket(struct addrinfo* addrInfo) {
    int socketFd = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
    if (socketFd == -1) {
        debugErr("socket", strerror(errno));
        freeaddrinfo(addrInfo);
        throw -1;
    }
    return socketFd;
}

void bindSocket(int socketFd, struct addrinfo* addrInfo) {
    int yes = 1;
    if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        debugErr("setsockopt", strerror(errno));
        freeaddrinfo(addrInfo);
        throw -1;
    }

    int status = bind(socketFd, addrInfo->ai_addr, addrInfo->ai_addrlen);
    if (status == -1) {
        debugErr("bind", strerror(errno));
        freeaddrinfo(addrInfo);
        throw -1;
    }
}

void listenToSocket(int socketFd, String port) {
    int status = listen(socketFd, 50);
    if (status == -1) {
        debugErr("listen", strerror(errno));
        throw -1;
    }
    // print specific <server_name>
    std::cout << "Server is listening on port " << port << "...\n";
}

int Server::listen() {
    struct addrinfo* serverInfo;

    try {
        std::vector<String> usedPorts;
        for (int i = 0; i < _serverBlockSize; i++) {
            if (std::find(usedPorts.begin(), usedPorts.end(), _blocks[i].getPort()) != usedPorts.end())
                continue;

            serverInfo = getServerAddressInfo(_blocks[i].getIpAddress(), _blocks[i].getPort());
            int socket = createSocket(serverInfo);
            bindSocket(socket, serverInfo);
            freeaddrinfo(serverInfo);
            listenToSocket(socket, _blocks[i].getPort());

            this->pushNewListeningSocket(socket);
            usedPorts.push_back(_blocks[i].getPort());
        }
    } catch (int error) {
        return error;
    }

    return this->monitorClients();
}

int Server::monitorClients() {
    while (true) {
        int eventsAmount = poll(&_fds[0], _fds.size(), -1);
        if (eventsAmount == -1) {
            debugErr("poll", strerror(errno));
            return -1;
        }
        this->scanForEventSockets(eventsAmount);
    }
    return 0;
}

void Server::scanForEventSockets(int eventsAmount) {
    int eventsFound = 0;
    _fdsSize = _fds.size();

    for (int _i = 0; _i < _fdsSize && eventsFound < eventsAmount; _i++) {
        int status = checkPollErrors(_fds[_i]);

        if (isReadable(_fds[_i]) && !status) {
            // debug("READABLE", std::to_string(_fds[_i].fd), GREEN);
            status = _pollFds[_i]->handleRead(*this, _pollFds[_i]);
            eventsFound++;
        } else if (isWritable(_fds[_i]) && !status) {
            // debug("WRITABLE", std::to_string(_fds[_i].fd), CYAN);
            status = _pollFds[_i]->handleWrite(*this, _pollFds[_i]);
            eventsFound++;
        }

        if (status == -1) {
            debug("removing", std::to_string(_fds[_i].fd), BOLD);
            this->removePollFd(_i);
            _i -= 1;
        }
    }
}

void Server::removePollFd(int index) {
    delete _pollFds[index];
    _pollFds.erase(_pollFds.begin() + index);
    _fds.erase(_fds.begin() + index);
    _fdsSize--;
}

void Server::pushNewClient(int clientSocket) {
    this->pushStructPollfd(clientSocket);

    ClientPoll* newClient = new ClientPoll(clientSocket);
    newClient->setReadHandler(readClientRequest);
    newClient->setWriteHandler(timeoutClient);
    _pollFds.push_back(newClient);
}

void Server::pushNewListeningSocket(int listeningSocket) {
    this->pushStructPollfd(listeningSocket);

    PollFd* newListeningPollFd = new PollFd(listeningSocket);
    newListeningPollFd->setReadHandler(handleNewConnection);
    _pollFds.push_back(newListeningPollFd);
}

CgiPoll& Server::pushNewCgiPoll(CgiSockets& cgiSockets, ClientPoll& client) {
    this->pushStructPollfd(cgiSockets.response[0]);
    // fcntl(cgiSockets.response[1], F_SETFL, O_NONBLOCK);
    // fcntl(cgiSockets.request[0], F_SETFL, O_NONBLOCK);
    // fcntl(cgiSockets.request[1], F_SETFL, O_NONBLOCK);

    CgiPoll* newCgi = new CgiPoll(cgiSockets, client, _fds[_fds.size() - 1]);
    newCgi->setWriteHandler(waitCgiProcessEnd);
    newCgi->setReadHandler(readCgiResponse);
    _pollFds.push_back(newCgi);
    return *newCgi;
}

void Server::pushStructPollfd(int fd) {
    fcntl(fd, F_SETFL, O_NONBLOCK);
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
        String body = "The server encountered some issue while handling your request";
        res.set(status, ".txt", &body);
    }
}

String Server::getDefaultErrorPagePath(int statusCode) {
    return "defaultPages/error/" + std::to_string(statusCode) + ".html";
}

ServerBlock& Server::findServerBlock(String port, String host) {
    String hostName = host.substr(0, host.find_first_of(':'));
    ServerBlock* defaultBlock = &_blocks[0];

    for (int i = 0; i < _serverBlockSize; i++) {
        if (_blocks[i].getPort() == port) {
            if (_blocks[i].isHost(hostName)) {
                return _blocks[i];
            }
            if (_blocks[i].isDefault()) {
                defaultBlock = &_blocks[i];
            }
        }
    }
    return *defaultBlock;
}

void Server::addBlocks(int size) {
    _blocks.resize(_serverBlockSize + size);
    _serverBlockSize += size;
}

void Server::addLocationBlocks(int serverIndex, int size) {
    _blocks[serverIndex].addLocationBlocks(size);
}

ServerBlock& Server::getServerBlock(int index) {
    return _blocks[index];
}

LocationBlock& Server::getLocationBlock(int serverIndex, int locationIndex) {
    return _blocks[serverIndex].getLocationBlock(locationIndex);
}

int handleNewConnection(Server& server, PollFd* listen) {
    int newClientSocket = accept(listen->getFd(), NULL, NULL);
    if (newClientSocket == -1) {
        debugErr("accept", strerror(errno));
        return 0;
    }
    debug("\nadding new client", std::to_string(newClientSocket), CYAN);
    server.pushNewClient(newClientSocket);
    return 0;
}
