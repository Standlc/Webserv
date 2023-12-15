#include "PollFd.hpp"

PollFd::PollFd(int fd, Server &server) : _status(new int(0)), _server(server) {
    _fd = fd;
    _readHandler = NULL;
    _writeHandler = NULL;
    _startTime = std::time(0);
    _concurrentReadWrite = true;
    _removeOnHungUp = true;
}

PollFd::~PollFd() {
    closeOpenFd(_fd);
};

void PollFd::destroy(int socketStatus) {
    _status = socketStatus;
}

void PollFd::setConcurrentReadWrite(bool value) {
    _concurrentReadWrite = value;
}

void PollFd::setRemoveOnHungUp(bool value) {
    _removeOnHungUp = value;
}

bool PollFd::removeOnHungUp() {
    return _removeOnHungUp;
}

bool PollFd::isConcurrentReadWrite() {
    return _concurrentReadWrite;
}

Server &PollFd::server() {
    return _server;
}

SharedPtr &PollFd::getStatus() {
    return _status;
}

time_t PollFd::startTime() {
    return _startTime;
}

void PollFd::resetStartTime() {
    _startTime = std::time(0);
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

int PollFd::handleWrite(PollFd *pollFd) {
    if (_writeHandler == NULL) {
        return 0;
    }
    return _writeHandler(pollFd);
}

int PollFd::handleRead(PollFd *pollFd) {
    if (_readHandler == NULL) {
        return 0;
    }
    return _readHandler(pollFd);
}