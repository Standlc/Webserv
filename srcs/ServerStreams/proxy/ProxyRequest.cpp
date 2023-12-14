#include "ProxyRequest.hpp"

ProxyRequest::ProxyRequest(HttpRequest &req, ProxyUrl &proxyPass) {
    _outputData = req.getHttpMethod() + " ";
    _outputData += proxyPass.path() + &req.rawUrl()[1] + " ";
    _outputData += req.getProtocol() + CRLF;

    _outputData += "Host: " + proxyPass.host() + CRLF;
    _outputData += "Connection: close" + String(CRLF);
    String headersToDiscard[] = {"Host", "Connection", ""};
    req.putHeaders(_outputData, headersToDiscard);
    _outputData += CRLF;
    _outputData += req.getBody();

    _outputDataSize = _outputData.size();
}

int ProxyRequest::send(int fd) {
    debugSending("sending proxy request", *this, fd, BLUE);
    return this->sendAll(fd, &_outputData[0], _outputDataSize);
}