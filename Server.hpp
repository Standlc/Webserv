#ifndef SERVER_HPP
#define SERVER_HPP

class Server;
class PollFd;
class ClientPollFd;
class CgiReadPollFd;
class CgiWritePollFd;

extern char** g_env;

#include "LocationBlock.hpp"
#include "PollFd.hpp"
#include "ServerBlock.hpp"
#include "webserv.hpp"

bool isReadable(struct pollfd& pollEl) {
    return (pollEl.revents & POLLIN) == 1;
}

bool isWritable(struct pollfd& pollEl) {
    return (pollEl.revents & POLLOUT);
}

int checkPollError(struct pollfd& pollEl, int error) {
    if (pollEl.revents & error) {
        debug("POLL ERROR", std::to_string(error), RED);
        debug("ON FD", std::to_string(pollEl.fd), RED);
        return error;
    }
    return 0;
}

int checkPollErrors(struct pollfd& pollEl) {
    if (checkPollError(pollEl, POLLERR) || checkPollError(pollEl, POLLHUP) || checkPollError(pollEl, POLLNVAL))
        return -1;
    return 0;
}

class Server {
   private:
    std::vector<struct pollfd> _fds;
    std::vector<PollFd*> _pollFds;
    std::vector<ServerBlock> _blocks;
    int _serverBlockSize;
    int _fdsSize;

   public:
    Server()
        : _serverBlockSize(0) {
    }
    ~Server() {
        for (size_t i = 0; i < _pollFds.size(); i++) {
            delete _pollFds[i];
        }
    }

    int listen() {
        std::vector<std::string> usedPorts;

        for (int i = 0; i < _serverBlockSize; i++) {
            if (std::find(usedPorts.begin(), usedPorts.end(), _blocks[i].getPort()) != usedPorts.end())
                continue;

            struct addrinfo* serverInfo = getOwnAddressInfo(&_blocks[i].getPort()[0]);
            if (serverInfo == NULL)
                return -1;

            int socket = createBindedSocket(serverInfo);
            if (socket == -1 || bindSocket(socket, serverInfo) == -1) {
                freeaddrinfo(serverInfo);
                return -1;
            }
            freeaddrinfo(serverInfo);

            if (listenToSocket(socket, _blocks[i].getPort()) == -1)
                return -1;

            this->pushNewListeningSocket(socket);
            usedPorts.push_back(_blocks[i].getPort());
        }
        return this->monitorClients();
    }

    int monitorClients() {
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

    void scanForEventSockets(int eventsAmount) {
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

    void removePollFd(int index) {
        delete _pollFds[index];
        _pollFds.erase(_pollFds.begin() + index);
        _fds.erase(_fds.begin() + index);
        _fdsSize--;
    }

    void pushNewClient(int clientSocket) {
        this->pushStructPollfd(clientSocket);

        ClientPollFd* newClient = new ClientPollFd(clientSocket);
        newClient->setReadHandler(readClientRequest);
        newClient->setWriteHandler(timeoutClient);
        _pollFds.push_back(newClient);
    }

    void pushNewListeningSocket(int listeningSocket) {
        this->pushStructPollfd(listeningSocket);

        PollFd* newListeningPollFd = new PollFd(listeningSocket);
        newListeningPollFd->setReadHandler(handleNewConnection);
        _pollFds.push_back(newListeningPollFd);
    }

    void pushNewCgiRequest(int pipes[2], ClientPollFd& client, LocationBlock& location, std::string cgiFile) {
        this->pushStructPollfd(pipes[1]);

        CgiWritePollFd* newCgi = new CgiWritePollFd(pipes, client, location, cgiFile);
        client.setCgiRequestStatus(newCgi->getStatus());
        newCgi->setWriteHandler(sendCgiRequest);
        _pollFds.push_back(newCgi);
    }

    void pushNewCgiResponse(int pid, int pipes[2], ClientPollFd& client) {
        this->pushStructPollfd(pipes[0]);

        CgiReadPollFd* newCgi = new CgiReadPollFd(pid, pipes, client);
        client.setCgiResponsStatus(newCgi->getStatus());
        newCgi->setReadHandler(readCgiResponseFromPipe);
        _pollFds.push_back(newCgi);
    }

    void pushStructPollfd(int fd) {
        fcntl(fd, F_SETFL, O_NONBLOCK);
        struct pollfd element;
        element.fd = fd;
        element.events = POLLIN | POLLOUT;
        element.revents = 0;
        _fds.push_back(element);
    }

    void loadDefaultErrPage(int statusCode, HttpResponse& res) {
        try {
            res.loadFile(statusCode, this->getDefaultErrorPagePath(statusCode));
        } catch (int status) {
            std::string body = "The server encountered some issue while handling your request";
            res.set(status, ".txt", &body);
        }
    }

    std::string getDefaultErrorPagePath(int statusCode) {
        return "defaultPages/error/" + std::to_string(statusCode) + ".html";
    }

    ServerBlock& findServerBlock(std::string port, std::string host) {
        std::string hostName = host.substr(0, host.find_first_of(':'));
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

    size_t findPollFd(int fd) {
        for (size_t i = 0; i < _pollFds.size(); i++) {
            if (_pollFds[i]->getFd() == fd) {
                return i;
            }
        }
        return -1;
    }

    void addBlocks(int size) {
        _blocks.resize(_serverBlockSize + size);
        _serverBlockSize += size;
    }

    void addLocationBlocks(int serverIndex, int size) {
        _blocks[serverIndex].addLocationBlocks(size);
    }

    ServerBlock& getServerBlock(int index) {
        return _blocks[index];
    }

    LocationBlock& getLocationBlock(int serverIndex, int locationIndex) {
        return _blocks[serverIndex].getLocationBlock(locationIndex);
    }
};

int sendResponseToClient(Server& server, ClientPollFd* client) {
    int clientSocket = client->getFd();

    // std::cout << "-------------------------\n";
    // std::cout << client->res().outputData() << "\n";
    // std::cout << "--------------------------------------------------\n";
    debug("sending response back to client", std::to_string(clientSocket), GREEN);
    if (client->res().sendResponse(clientSocket) <= 0) {
        return -1;
    }
    return 0;
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

int readClientRequest(Server& server, ClientPollFd* client) {
    HttpRequest& req = client->req();
    HttpResponse& res = client->res();

    debug("reading client request", std::to_string(client->getFd()), WHITE);
    if (req.recvAll(client->getFd()) == -1) {
        return -1;
    }

    try {
        if (req.isComplete() == false && req.resumeParsing() == true) {
            debug("parsing request", std::to_string(client->getFd()), WHITE);
            // std::cout << "--------------------------------------------------\n";
            // std::cout << req.rawData() << "\n";
            // std::cout << "-------------------------\n";
            // std::cout << req.transferDecodedBody() << "\n";
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

int timeoutClient(Server& server, ClientPollFd* client) {
    return checkTimeout(client->getAcceptTime());
}

int executeClientRequest(Server& server, ClientPollFd* client) {
    clientPollFdHandlerType writeHandler = NULL;

    try {
        std::string socketPort = getSocketPort(client->getFd());
        ServerBlock& block = server.findServerBlock(socketPort, client->req().getHeader("Host"));
        writeHandler = block.execute(server, *client);

    } catch (int statusCode) {
        server.loadDefaultErrPage(statusCode, client->res());
        writeHandler = sendResponseToClient;
    }

    client->setWriteHandler(writeHandler);
    return writeHandler(server, client);
}

int readCgiResponseFromPipe(Server& server, CgiReadPollFd* cgi) {
    if (cgi->getClientStatus() == -1) {
        debug("cgi client error", std::to_string(cgi->getFd()), RED);
        kill(cgi->getPid(), SIGKILL);
        return -1;
    }

    ClientPollFd& client = cgi->client();

    debug("reading cgi response from pipe", std::to_string(cgi->getFd()), WHITE);
    if (client.res().recvAll(cgi->getFd()) == -1) {
        kill(cgi->getPid(), SIGKILL);
        return -1;
    }
    client.setWriteHandler(waitEmptyCgiPipe);
    return 0;
}

int setCgiResponse(Server& server, ClientPollFd* client) {
    HttpResponse& res = client->res();

    if (client->cgiResponseStatus() == -1) {
        server.loadDefaultErrPage(500, res);
    } else {
        int* pipes = client->cgiResponsePipes();
        debug("removing cgi pipe", std::to_string(pipes[0]), WHITE);
        int CgiReadPollFdIndex = server.findPollFd(pipes[0]);
        server.removePollFd(CgiReadPollFdIndex);
        res.setCgiResponse(200);
    }

    client->setWriteHandler(sendResponseToClient);
    return sendResponseToClient(server, client);
}

int waitEmptyCgiPipe(Server& server, ClientPollFd* client) {
    debug("waiting for cgi to empty the pipe", std::to_string(client->getFd()), WHITE);
    client->setWriteHandler(setCgiResponse);
    return 0;
}

int handleCgiProcessErr(Server& server, ClientPollFd* client, int* pipes) {
    close(pipes[0]);
    close(pipes[1]);
    server.loadDefaultErrPage(500, client->res());
    client->setWriteHandler(sendResponseToClient);
    return sendResponseToClient(server, client);
}

int checkTimeout(time_t time) {
    if (std::time(0) - time > TIMEOUT) {
        debug("timing out", "", DIM_RED);
        return -1;
    }
    return 0;
}

int waitCgiProcessEnd(Server& server, ClientPollFd* client) {
    int cgiPid = client->cgiPid();
    int* pipes = client->cgiResponsePipes();
    int waitpidStatus;

    // debug("checking cgi pid", std::to_string(cgiPid), WHITE);
    if (waitpid(cgiPid, &waitpidStatus, WNOHANG) == -1) {
        if (WIFEXITED(waitpidStatus) && WEXITSTATUS(waitpidStatus) != 0) {
            debug("cgi process has exit with error", std::to_string(cgiPid), RED);
            return handleCgiProcessErr(server, client, pipes);
        }

        debug("cgi process has exit", std::to_string(cgiPid), WHITE);
        server.pushNewCgiResponse(client->cgiPid(), pipes, *client);
        client->setWriteHandler(waitEmptyCgiPipe);
    }
    if (checkTimeout(client->cgiLaunchTime()) == -1) {
        kill(SIGKILL, cgiPid);
        return handleCgiProcessErr(server, client, pipes);
    }
    return 0;
}

int sendCgiRequest(Server& server, CgiWritePollFd* cgi) {
    if (cgi->clientStatus() == -1) {
        return -1;
    }

    ClientPollFd& client = cgi->client();
    int* pipes = cgi->getPipes();

    debug("sending cgi request", std::to_string(pipes[1]), WHITE);
    int sendStatus = client.req().sendBody(pipes[1]);
    if (sendStatus <= 0) {
        if (sendStatus == 0 && cgi->getLocation().execveCgi(server, client, pipes, cgi->getCgiFile()) != -1) {
            client.setWriteHandler(waitCgiProcessEnd);
        }
        return -1;
    }
    return 0;
}

int checkCgiWrite(Server& server, ClientPollFd* client) {
    if (client->cgiRequestStatus() == -1) {
        server.loadDefaultErrPage(500, client->res());
        client->setWriteHandler(sendResponseToClient);
        return sendResponseToClient(server, client);
    }
    return 0;
}

////////////////////////////////////////////////////////////

clientPollFdHandlerType LocationBlock::execute(Server& server, ClientPollFd& client) {
    HttpResponse& res = client.res();
    HttpRequest& req = client.req();

    try {
        res.addHeaders(_headers);

        if (_redirection.url != "") {
            res.addHeader("Location", this->assembleRedirectionUrl(req));
            throw _redirection.statusCode;
        }

        // proxyPass()

        this->throwReqErrors(req);

        if (this->handlesCgiExtension(getFileExtension(req.getUrl()))) {
            return handleCgi(server, client);
        } else {
            _handlers[req.getHttpMethod()](*this, req, res);
        }

        if (_sessionCookieName != "" && req.findCookie(_sessionCookieName) == "") {
            res.addHeader("Set-Cookie", _sessionCookieName + "=" + this->generateSessionCookie());
        }
    } catch (int status) {
        if (status >= 400) {
            res.clearHeaders();
        }
        this->loadErrPage(status, res, req);
    }
    return sendResponseToClient;
}

clientPollFdHandlerType LocationBlock::handleCgi(Server& server, ClientPollFd& client) {
    int fileAccessStatus = checkPathAccess(this->getResourcePath(client.req()));
    if (fileAccessStatus != 200) {
        throw fileAccessStatus;
    }

    std::string cgiFile = "." + client.req().getUrl();
    if (client.req().hasBody()) {
        int pipes[2];
        if (pipe(pipes) == -1) {
            debugErr("pipe", strerror(errno));
            throw 500;
        }
        server.pushNewCgiRequest(pipes, client, *this, cgiFile);
        return checkCgiWrite;
    }

    this->execveCgi(server, client, NULL, cgiFile);
    return waitCgiProcessEnd;
}

#endif