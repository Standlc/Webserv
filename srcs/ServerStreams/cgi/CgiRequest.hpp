#ifndef CGI_REQUEST_HPP
#define CGI_REQUEST_HPP

#include "../HttpRequest.hpp"
#include "../ServerStream.hpp"

class CgiRequest : public ServerStream {
   private:
    const String &_body;

   public:
    CgiRequest(HttpRequest &req) : _body(req.getBody()) {
        _outputDataSize = req.getBodySize();
    }

    int send(int fd) {
        debugSending("sending CGI request", *this, fd, PURPLE);
        return this->sendAll(fd, (char *)&_body[0], _outputDataSize);
    }
};

#endif