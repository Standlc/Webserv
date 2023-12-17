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
        _clientRes.addDefaultHeaders(_headers);

        // ??
        // _clientRes.setBody(_body);
        _clientRes.setBody(&_rawData[_endOfHeadersPos]);
        // _clientRes.setBody(_rawData.substr(_endOfHeadersPos, _totalRead - _endOfHeadersPos));
        _clientRes.set(_responseStatus);
    }
};

#endif
