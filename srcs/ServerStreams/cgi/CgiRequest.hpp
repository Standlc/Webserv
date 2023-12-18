#ifndef CGI_REQUEST_HPP
#define CGI_REQUEST_HPP

#include "../HttpRequest.hpp"
#include "../ServerStream.hpp"

class CgiRequest : public ServerStream {
   private:
    const String &_body;

   public:
    CgiRequest(HttpRequest &req);
    int send(int fd);
};

#endif