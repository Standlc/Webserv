#ifndef CGI_REQUEST_HPP
#define CGI_REQUEST_HPP

#include "../HttpRequest.hpp"
#include "../ServerStream.hpp"

class CgiRequest : public ServerStream {
   private:
    const String &_body;
    size_t _bodySize;

   public:
    CgiRequest(HttpRequest &req) : _body(req.getBody()) {
        _bodySize = req.getBodySize();
    }

    int send(int fd) {
        debugMessageInfos("sending cgi request to", fd, _bodySize, YELLOW);
        return this->sendAll(fd, (char *)&_body[0], _bodySize);
    }
};

#endif