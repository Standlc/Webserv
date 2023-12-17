#ifndef PROXY_RESPONSE_HPP
#define PROXY_RESPONSE_HPP

#include "../HttpParser.hpp"
#include "../HttpResponse.hpp"

class ProxyResponse : public HttpParser {
   private:
    HttpResponse &_clientRes;

   public:
    ProxyResponse(HttpResponse &res) : _clientRes(res) {
    }

    bool resumeParsing() {
        if (this->parseResponseHead() == true) {
            return HttpParser::resumeParsing(false);
        }
        return false;
    }

    void setClientRes() {
        _headers.erase("Server");
        _headers.erase("Connection");
        _headers.erase("Content-Length");
        _headers.add("Content-Length", std::to_string(_bodySize));
        _clientRes.addDefaultHeaders(_headers);

        _clientRes.setBody(_rawData.substr(_endOfHeadersPos, _bodySize));
        _clientRes.set(_responseStatus);
    }
};

#endif
