#include "CgiRequest.hpp"

CgiRequest::CgiRequest(HttpRequest &req) : _body(req.getBody()) {
    _outputDataSize = req.getBodySize();
}

int CgiRequest::send(int fd) {
    debugSending("sending CGI request", *this, fd, PURPLE);
    return this->sendAll(fd, (char *)&_body[0], _outputDataSize);
}