#ifndef PROXY_RESPONSE_HPP
#define PROXY_RESPONSE_HPP

#include "../HttpParser.hpp"
#include "../HttpResponse.hpp"

class ProxyResponse : public HttpParser {
   private:
    HttpResponse &_clientRes;

   public:
    ProxyResponse(HttpResponse &res);
    bool resumeParsing();
    void setClientRes();
};

#endif
