#include "PollFd.hpp"

CgiPoll::CgiPoll(int fd, int *pipes, ClientPoll &client) : PollFd(fd),
                                                           _client(client) {
    _pipes[0] = pipes[0];
    _pipes[1] = pipes[1];
    _clientStatus = client.getStatus();
    client.setCgiPollStatus(_status);
}

CgiPoll::~CgiPoll() {
    if (_pipes[0] != _fd) {
        close(_pipes[0]);
    }
    if (_pipes[1] != _fd) {
        close(_pipes[1]);
    }
}

int CgiPoll::clientStatus() {
    if (_clientStatus != NULL) {
        return *_clientStatus;
    }
    return 0;
}

int *CgiPoll::getPipes() {
    return _pipes;
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

//////////////////////////////////////////////////////////////////

int readCgiResponseFromPipe(Server &server, CgiPoll *cgi) {
    if (cgi->clientStatus() == -1) {
        return -1;
    }

    ClientPoll &client = cgi->client();
    debug("reading cgi response from pipe", std::to_string(cgi->getFd()), WHITE);
    if (client.res().recvAll(cgi->getFd()) == -1) {
        return -1;
    }
    client.setWriteHandler(waitEmptyCgiPipe);
    return 0;
}

int sendCgiRequest(Server &server, CgiPoll *cgi) {
    if (cgi->clientStatus() == -1) {
        return -1;
    }

    ClientPoll &client = cgi->client();
    int *pipes = cgi->getPipes();

    debug("sending cgi request", std::to_string(pipes[1]), WHITE);
    int sendStatus = client.req().sendBody(pipes[1]);
    if (sendStatus == 0) {
        client.setWriteHandler(waitCgiProcessEnd);
        return -1;
    }
    return (sendStatus == -1) ? -1 : 0;
}
