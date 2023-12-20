#include "ProxyResponse.hpp"

ProxyResponse::ProxyResponse(HttpResponse &res) : _clientRes(res) {
}

bool ProxyResponse::resumeParsing() {
    if (this->parseResponseHead() == true) {
        return HttpParser::resumeParsing(false);
    }
    return false;
}

void ProxyResponse::setClientRes() {
    _headers.erase("Server");
    _headers.erase("Connection");
    _headers.erase("Content-Length");
    _headers.add("Content-Length", toString(_bodySize));
    _clientRes.addDefaultHeaders(_headers);

    if (_endOfHeadersPos != NPOS && _bodySize > 0) {
        _clientRes.setBody(_rawData.substr(_endOfHeadersPos, _bodySize));
    } else {
        _clientRes.setBody("");
    }
    _clientRes.set(_responseStatus);
}