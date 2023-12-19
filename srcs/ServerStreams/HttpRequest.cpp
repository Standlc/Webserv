#include "HttpRequest.hpp"

HttpRequest::HttpRequest(int clientSocket) {
    _socket = clientSocket;
};

bool HttpRequest::resumeParsing() {
    if (this->parseRequestHead() == true) {
        return HttpParser::resumeParsing(true);
    }
    return false;
}

String HttpRequest::findBodyHeader(String key, size_t from, size_t to) {
    key = CRLF + key + ":";
    size_t start = _body.find(key, from);
    if (start == NPOS || start > to) {
        return "";
    }
    start += key.length();

    size_t end = _body.find(CRLF, start);
    if (end == NPOS) {
        return "";
    }

    start += countFrontSpaces(_body, start);
    end -= countBackSpaces(_body, end);
    return _body.substr(start, end - start);
}

size_t HttpRequest::searchBody(const String &find, size_t from, size_t upto) {
    size_t pos = _body.find(find, from);
    if (upto != NPOS && pos != NPOS) {
        return (pos <= upto) ? pos : -1;
    }
    return pos;
}

String HttpRequest::getClientHostName() {
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    if (getpeername(_socket, (struct sockaddr *)&addr, &addr_len) == -1) {
        debugErr("getpeername", strerror(errno));
        throw 500;
    }

    char host[NI_MAXHOST];
    if (getnameinfo((struct sockaddr *)&addr, addr_len, host, NI_MAXHOST, NULL, 0, 0) == -1) {
        debugErr("getnameinfo", strerror(errno));
        throw 500;
    }
    return host;
}

String HttpRequest::getClientIpAddress() {
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    if (getpeername(_socket, (struct sockaddr *)&addr, &addr_len) == -1) {
        debugErr("getpeername", strerror(errno));
        throw 500;
    }

    char ipAddress[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr.sin_addr), ipAddress, INET_ADDRSTRLEN);
    return String(ipAddress);
}

String HttpRequest::getSocketIpAddress() {
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);

    if (getsockname(_socket, (struct sockaddr *)&sin, &len) == -1) {
        debugErr("getsockname", strerror(errno));
        throw 500;
    }

    char ipAddress[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(sin.sin_addr), ipAddress, INET_ADDRSTRLEN);
    return String(ipAddress);
}

String HttpRequest::getSocketPort() {
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);

    if (getsockname(_socket, (struct sockaddr *)&sin, &len) == -1) {
        debugErr("getsockname", strerror(errno));
        throw 500;
    }
    return toString(ntohs(sin.sin_port));
}

void HttpRequest::setUrl(const String &url) {
    _rawUrl = url;
    this->processUrl(_rawUrl);
}

void HttpRequest::putHeaders(String &buf, String headersToDiscard[]) {
    _headers.putIn(buf, headersToDiscard);
}

void HttpRequest::putHead(String &buf) {
    buf = _httpMethod + " ";
    buf += _rawUrl + " ";
    buf += _httpProtocol + CRLF;
}

int HttpRequest::socket() {
    return _socket;
}