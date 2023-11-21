#include "HttpRequest.hpp"

HttpRequest::HttpRequest(int clientSocket) {
    _clientSocket = clientSocket;
    _endOfHeadersPos = -1;
    _contentLengthHeader = 0;
    _isEncodingChunked = -1;
    _bodySize = 0;
    _isComplete = false;
    _currChunkSize = -1;
    _encodedBytesLeftToRead = 0;
    _decodedBodySize = 0;
    _parsingPos = 0;
};

size_t HttpRequest::parseHead() {
    size_t endOfHead = _rawData.find(CRLF);
    if (endOfHead == NPOS) {
        throw 400;
    }

    const std::vector<String> &headElements = split(_rawData.substr(0, endOfHead), " \t");
    if (headElements.size() != 3 ||
        (headElements[1][0] != '/' || headElements[1].find("../") != NPOS) ||
        (headElements[2] != "HTTP/1.0" && headElements[2] != "HTTP/1.1")) {
        throw 400;
    }

    _httpMethod = headElements[0];
    this->processUrl(headElements[1]);
    _httpProtocol = headElements[2];
    return endOfHead + 2;
}

void HttpRequest::processUrl(const String &url) {
    size_t paramPos = url.find('?');
    _url.path = url.substr(0, paramPos);
    compressSlashes(_url.path);
    percentDecode(_url.path);

    if (paramPos == NPOS) {
        _url.params = "";
    } else {
        _url.params = url.substr(paramPos + 1, -1);
    }
}

size_t HttpRequest::parseHeaders() {
    size_t currPos = _parsingPos;
    size_t pos = 0;

    while (currPos < _totalRead) {
        pos = _rawData.find(CRLF, currPos);
        if (pos == NPOS) {
            throw 400;
        }

        if (pos == currPos) {
            _contentLengthHeader = atol(&this->getHeader("Content-Length")[0]);
            _isEncodingChunked = this->getHeader("Transfer-Encoding") == "chunked";
            _endOfHeadersPos = pos + 2;
            break;
        }

        this->parseHeaderLine(_rawData.substr(currPos, pos - currPos));
        currPos = pos + 2;
    }

    return pos + 2;
}

void HttpRequest::parseHeaderLine(const String &headerLine) {
    int headerLen = headerLine.length();
    int endOfKey = headerLine.find_first_of(':');
    if (endOfKey == -1) {
        return;
    }
    String key = headerLine.substr(0, endOfKey);
    endOfKey += 1;

    size_t valueStart = endOfKey + countFrontSpaces(headerLine, "", endOfKey);
    size_t valueEnd = headerLen - countBackSpaces(headerLine, "", headerLen);
    size_t size = (valueEnd < valueStart) ? 0 : (valueEnd - valueStart);
    String value = headerLine.substr(valueStart, size);

    _headers.insert(std::make_pair(key, value));
}

bool HttpRequest::readChunk() {
    // debug("reading chunk", "", GRAY);
    size_t i = _parsingPos;
    while (i < _totalRead && i < _parsingPos + _encodedBytesLeftToRead) {
        i++;
    }

    if (i == _totalRead) {
        _body += _rawData.substr(_parsingPos);
        _encodedBytesLeftToRead -= _totalRead - _parsingPos;
        _parsingPos = _totalRead;
        return false;
    }

    if (_rawData.compare(i, 2, CRLF) != 0) {
        throw 400;
    }

    _body += _rawData.substr(_parsingPos, _encodedBytesLeftToRead);
    _parsingPos = i + 2;

    if (_currChunkSize == 0 && _decodedBodySize > 0) {
        return true;
    }

    _currChunkSize = NPOS;
    return false;
}

void HttpRequest::readChunkSize() {
    // debug("reading chunk size", "", GRAY);
    size_t crlfPos = _rawData.find(CRLF, _parsingPos);
    if (crlfPos == NPOS) {
        throw 400;
    }

    try {
        _currChunkSize = std::stol(_rawData.substr(_parsingPos, crlfPos - _parsingPos), NULL, 16);
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        throw 400;
    }

    if (_currChunkSize < 0) {
        throw 400;
    }

    debug("_currChunkSize", std::to_string(_currChunkSize), YELLOW);
    _decodedBodySize += _currChunkSize;
    _parsingPos = crlfPos + 2;
}

bool HttpRequest::decodeBody() {
    while (_parsingPos < _totalRead) {
        if (_currChunkSize == -1) {
            this->readChunkSize();
            _encodedBytesLeftToRead = _currChunkSize;
        }

        if (_parsingPos == _totalRead) {
            break;
        }

        if (this->readChunk() == true) {
            return true;
        }
    }
    return false;
}

const String &HttpRequest::getHttpMethod() {
    return _httpMethod;
}

const String &HttpRequest::getProtocol() {
    return _httpProtocol;
}

const URL &HttpRequest::url() {
    return _url;
}

bool HttpRequest::hasBody() {
    return _bodySize > 0;
}

bool HttpRequest::isComplete() {
    return _isComplete;
}

const String &HttpRequest::getBody() {
    return _body;
}

bool HttpRequest::resumeParsing() {
    try {
        if (_parsingPos == 0) {
            debug("parsing headline", "", GRAY);
            _parsingPos = this->parseHead();
        }

        if (_endOfHeadersPos == NPOS && _parsingPos < _totalRead) {
            debug("parsing headers", "", GRAY);
            _parsingPos = this->parseHeaders();
        }

        if (_parsingPos >= _endOfHeadersPos) {
            debug("parsing body", "", GRAY);

            if (_isEncodingChunked) {
                if ((_isComplete = this->decodeBody()) == true) {
                    _bodySize = _body.size();
                }
            } else if (_contentLengthHeader > 0) {
                if ((_totalRead - _endOfHeadersPos) >= _contentLengthHeader) {
                    _bodySize = _contentLengthHeader;
                    _body = _rawData.substr(_endOfHeadersPos, _bodySize);
                    _isComplete = true;
                }
            } else {
                _bodySize = _totalRead - _endOfHeadersPos;
                _body = _rawData.substr(_endOfHeadersPos, _bodySize);
                _isComplete = true;
            }
        }
    } catch (int status) {
        _isComplete = true;
        throw status;
    }
    return _isComplete;
}

String HttpRequest::getHeader(const String &key) {
    std::unordered_map<String, String>::iterator header = _headers.find(key);
    if (header == _headers.end()) {
        return "";
    }
    return header->second;
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

    start += countFrontSpaces(_body, "", start);
    end -= countBackSpaces(_body, "", end);
    return _body.substr(start, end - start);
}

String HttpRequest::findCookie(String cookieName) {
    cookieName += "=";
    String cookieField = this->getHeader("Cookie");
    if (cookieField == "") {
        return "";
    }

    std::vector<String> cookies = split(cookieField, ";");
    for (size_t i = 0; i < cookies.size(); i++) {
        if (cookies[i].compare(0, cookieName.length(), cookieName) == 0) {
            return cookies[i].substr(cookieName.length());
        }
    }
    return "";
}

size_t HttpRequest::getBodySize() {
    return _bodySize;
}

int HttpRequest::sendBody(int fd) {
    return this->sendAll(fd, &_body[0], _bodySize);
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

    if (getpeername(_clientSocket, (struct sockaddr *)&addr, &addr_len) == -1) {
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

    if (getpeername(_clientSocket, (struct sockaddr *)&addr, &addr_len) == -1) {
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

    if (getsockname(_clientSocket, (struct sockaddr *)&sin, &len) == -1) {
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

    if (getsockname(_clientSocket, (struct sockaddr *)&sin, &len) == -1) {
        debugErr("getsockname", strerror(errno));
        throw 500;
    }
    return std::to_string(ntohs(sin.sin_port));
}