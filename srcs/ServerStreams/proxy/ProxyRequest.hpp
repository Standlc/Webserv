#ifndef PROXY_REQUEST_HPP
#define PROXY_REQUEST_HPP

#include "../../blocks/LocationBlock/utils.hpp"
#include "../HttpRequest.hpp"
#include "../ServerStream.hpp"

class ProxyRequest : public ServerStream {
   public:
    ProxyRequest(HttpRequest &req, ProxyUrl &proxyPass);
    int send(int fd);
};

#endif
