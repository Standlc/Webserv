#include "PollFd.hpp"

PollFd::PollFd(int fd) : _status(new int(0)) {
    _fd = fd;
    _readHandler = NULL;
    _writeHandler = NULL;
}

PollFd::~PollFd() {
    closeOpenFd(_fd);
    *_status = -1;
};

std::shared_ptr<int> PollFd::getStatus() {
    return _status;
}

int PollFd::getFd() {
    return _fd;
}

void PollFd::setWriteHandler(pollFdHandlerType f) {
    _writeHandler = f;
}

void PollFd::setReadHandler(pollFdHandlerType f) {
    _readHandler = f;
}

int PollFd::handleWrite(Server &server, PollFd *pollFd) {
    if (_writeHandler == NULL)
        return 0;
    return _writeHandler(server, pollFd);
}

int PollFd::handleRead(Server &server, PollFd *pollFd) {
    if (_readHandler == NULL)
        return 0;
    return _readHandler(server, pollFd);
}