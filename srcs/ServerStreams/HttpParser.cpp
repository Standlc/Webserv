#include "HttpParser.hpp"

bool HttpParser::resumeParsing(bool copyBody) {
    try {
        if (_endOfHeadersPos == NPOS && _parsingPos < _totalRead) {
            debug("parsing headers", "", GRAY);
            _parsingPos = this->parseHeaders();
        }
        if (_parsingPos >= _endOfHeadersPos && _endOfHeadersPos != NPOS) {
            debug("parsing body", "", GRAY);
            _isComplete = this->handleBodyParsing(copyBody);
        }
    } catch (int parsingErr) {
        _isComplete = true;
        throw parsingErr;
    }
    return _isComplete;
}

bool HttpParser::discardEmptyLine(int *sizeCRLF) {
    _headerLineEndPos = findCRLF(sizeCRLF, _parsingPos);
    if (_headerLineEndPos == NPOS) {
        return true;
    }
    if (_headerLineEndPos == _parsingPos) {
        _parsingPos += *sizeCRLF;
        _headerLineEndPos = NPOS;
        return true;
    }
    return false;
}

bool HttpParser::parseRequestHead() {
    if (_headerLineEndPos != NPOS) {
        return true;
    }
    int sizeCRLF = 0;
    if (discardEmptyLine(&sizeCRLF)) {
        return false;
    }
    debug("parsing request head", "", GRAY);

    String headLine = _rawData.substr(0, _headerLineEndPos);
    const std::vector<String> &headElements = split(headLine, " \t");
    if (headElements.size() != 3 ||
        (headElements[2] != "HTTP/1.0" && headElements[2] != "HTTP/1.1")) {
        throw 1;
    }

    _httpMethod = headElements[0];
    _rawUrl = headElements[1];
    this->processUrl(headElements[1]);
    _httpProtocol = headElements[2];
    _parsingPos = _headerLineEndPos + sizeCRLF;
    return true;
}

bool HttpParser::parseResponseHead() {
    if (_headerLineEndPos != NPOS) {
        return true;
    }
    int sizeCRLF = 0;
    if (discardEmptyLine(&sizeCRLF)) {
        return false;
    }
    debug("parsing response head", "", GRAY);

    String headLine = _rawData.substr(0, _headerLineEndPos);
    const std::vector<String> &headElements = split(headLine, " \t", 2);
    if (headElements.size() != 2 ||
        (headElements[0] != "HTTP/1.0" && headElements[0] != "HTTP/1.1")) {
        throw 1;
    }

    _httpProtocol = headElements[0];
    try {
        _responseStatus = toInt(headElements[1]);
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        throw 1;
    }
    _parsingPos = _headerLineEndPos + sizeCRLF;
    return true;
}

void HttpParser::processUrl(const String &url) {
    throwIf(url.find("../") != NPOS || url[0] != '/', 400);

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

void HttpParser::getBodyInfos() {
    const String &contentLengthHeader = _headers.find("Content-Length");
    if (contentLengthHeader != "") {
        try {
            _contentLengthHeader = toInt(&contentLengthHeader[0]);
        } catch (const std::exception &e) {
            std::cerr << e.what() << '\n';
            throw 1;
        }
    }
    _isEncodingChunked = _headers.find("Transfer-Encoding") == "chunked";
}

size_t HttpParser::parseHeaders() {
    int sizeCRLF = 0;
    size_t startPos = _parsingPos;
    size_t endPos = 0;

    while ((endPos = findCRLF(&sizeCRLF, startPos)) != NPOS) {
        bool reachHeadersEnd = startPos == endPos;
        if (reachHeadersEnd) {
            this->getBodyInfos();
            _endOfHeadersPos = endPos + sizeCRLF;
            return _endOfHeadersPos;
        }

        String headerLine = _rawData.substr(startPos, endPos - startPos);
        this->parseHeaderLine(headerLine);
        startPos = endPos + sizeCRLF;
    }

    return startPos;
}

void HttpParser::parseHeaderLine(const String &line) {
    int headerLen = line.length();
    size_t endOfKey = line.find(':');
    if (endOfKey == NPOS || !endOfKey || std::isspace(line[endOfKey - 1])) {
        throw 1;
    }
    String key = line.substr(0, endOfKey);
    endOfKey += 1;

    size_t valueStart = endOfKey + countFrontSpaces(line, endOfKey);
    size_t valueEnd = headerLen - countBackSpaces(line, headerLen);
    size_t size = (valueEnd <= valueStart) ? 0 : (valueEnd - valueStart);

    if (size > 0) {
        String value = line.substr(valueStart, size);
        _headers.add(key, value);
    }
}

bool HttpParser::handleBodyParsing(bool parseBody) {
    if (_isEncodingChunked) {
        if (this->resumeDecodingBody(parseBody) == true) {
            return true;
        }
        return false;
    }

    if (_contentLengthHeader >= 0) {
        if ((long)(_totalRead - _endOfHeadersPos) >= _contentLengthHeader) {
            _bodySize = _contentLengthHeader;
            this->appendBody(parseBody, _endOfHeadersPos, _bodySize);
            return true;
        }
        return false;
    }

    _bodySize = _totalRead - _endOfHeadersPos;
    this->appendBody(parseBody, _endOfHeadersPos, _bodySize);
    return parseBody;
}

bool HttpParser::resumeDecodingBody(bool copyBody) {
    while (_parsingPos < _totalRead) {
        if (_currChunkSize == -1) {
            // debug("reading chunk size");
            if (this->readChunkSize() == false) {
                return false;
            }
            _encodedBytesLeftToRead = _currChunkSize;
        }
        if (_parsingPos == _totalRead) {
            break;
        }
        // debug("reading chunk");
        if (this->readChunk(copyBody) == true) {
            return true;
        }
    }
    return false;
}

void HttpParser::appendBody(bool copyBody, size_t from, size_t size) {
    if (copyBody) {
        _body += _rawData.substr(from, size);
    }
}

bool HttpParser::readChunk(bool copyBody) {
    size_t i = _parsingPos;
    while (i < _totalRead && i < _parsingPos + _encodedBytesLeftToRead) {
        i++;
    }

    if (i == _totalRead) {
        appendBody(copyBody, _parsingPos, -1);
        _encodedBytesLeftToRead -= _totalRead - _parsingPos;
        _parsingPos = _totalRead;
        return false;
    }

    int sizeCRLF = 0;
    if (findCRLF(&sizeCRLF, i) == NPOS) {
        throw 1;
    }

    appendBody(copyBody, _parsingPos, _encodedBytesLeftToRead);
    _parsingPos = i + sizeCRLF;

    if (_currChunkSize == 0 && _bodySize > 0) {
        return true;
    }
    _currChunkSize = -1;
    return false;
}

bool HttpParser::readChunkSize() {
    int sizeCRLF;
    size_t chunkSizeEndPos = findCRLF(&sizeCRLF, _parsingPos);
    if (chunkSizeEndPos == NPOS) {
        return false;
    }

    try {
        String chunkSize = _rawData.substr(_parsingPos, chunkSizeEndPos - _parsingPos);
        _currChunkSize = hexToInt(chunkSize);
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        throw 1;
    }

    if (_currChunkSize < 0) {
        throw 1;
    }
    _bodySize += _currChunkSize;
    _parsingPos = chunkSizeEndPos + sizeCRLF;
    return true;
}

size_t HttpParser::findCRLF(int *sizeCRLF, size_t from) {
    size_t pos = _rawData.find_first_of(CRLF, from);
    if (pos == NPOS) {
        return NPOS;
    }

    if (_rawData[pos] == '\r') {
        *sizeCRLF = 2;
        throwIf(_rawData[pos + 1] != '\n', 1);
    } else {
        *sizeCRLF = 1;
    }
    return pos;
}