#ifndef PROXY_REQUEST_HPP
#define PROXY_REQUEST_HPP

#include "../HttpRequest.hpp"
#include "../ServerStream.hpp"

class ProxyRequest : public ServerStream {
   private:
    size_t _outputDataSize;
    String _outputData;

   public:
    ProxyRequest(HttpRequest &req, const String &remoteHostName);
    int send(int fd);
};

#endif
