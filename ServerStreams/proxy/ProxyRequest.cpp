#include "ProxyRequest.hpp"

ProxyRequest::ProxyRequest(HttpRequest &req, const String &remoteHostName) {
    req.putHead(_outputData);
    _outputData += "Host: " + remoteHostName + CRLF;
    _outputData += "Connection: close" + String(CRLF);
    req.putHeaders(_outputData, (char *[]){(char *)"Host", (char *)"Connection", NULL});
    _outputData += CRLF;
    _outputData += req.getBody();
    _outputDataSize = _outputData.size();
}

int ProxyRequest::send(int fd) {
    debugMessageInfos("sending proxy request", fd, _outputDataSize, BLUE);
    debugHttpMessage(_outputData, BLUE);
    return this->sendAll(fd, &_outputData[0], _outputDataSize);
}