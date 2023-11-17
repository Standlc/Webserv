#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include "ServerStream.hpp"
#include "webserv.hpp"

void compressSlashes(std::string &str) {
    int startSlashPos = str.find_first_of("/");
    int endSlashPos = str.find_first_not_of("/", startSlashPos);

    while (startSlashPos != -1) {
        str.replace(startSlashPos, endSlashPos != -1 ? endSlashPos - startSlashPos : -1, "/");
        startSlashPos = str.find_first_of("/", startSlashPos + 1);
        endSlashPos = str.find_first_not_of("/", startSlashPos);
    }
}

void uriDecode(std::string &url, std::string find, std::string replace) {
    int pos = url.find(find);

    while (pos != -1) {
        url.replace(pos, 3, replace);
        pos = url.find(find, pos + 1);
    }
}

void percentDecode(std::string &url) {
    uriDecode(url, "%20", " ");
    uriDecode(url, "%21", "!");
    uriDecode(url, "%22", "\"");
    uriDecode(url, "%23", "#");
    uriDecode(url, "%24", "$");
    uriDecode(url, "%25", "%");
    uriDecode(url, "%26", "&");
    uriDecode(url, "%27", "\"");
    uriDecode(url, "%28", "(");
    uriDecode(url, "%29", ")");

    uriDecode(url, "%2A", "*");
    uriDecode(url, "%2B", "+");
    uriDecode(url, "%2C", ",");
    uriDecode(url, "%2F", "/");

    uriDecode(url, "%3A", ":");
    uriDecode(url, "%3B", ";");
    uriDecode(url, "%3D", "=");
    uriDecode(url, "%3F", "?");

    uriDecode(url, "%40", "@");

    uriDecode(url, "%5B", "[");
    uriDecode(url, "%5D", "]");
}

size_t countBackSpaces(const std::string &str, const std::string &sep, size_t end) {
    if (end == 0) {
        return 0;
    }

    std::string spaces = " \t";
    end--;
    int nSpaces = 0;
    char c = str[end];

    while (spaces.find(c) != (size_t)-1 && sep.find(c) == (size_t)-1 && c != '\"') {
        nSpaces++;
        if (end - nSpaces == 0) {
            break;
        }
        c = str[end - nSpaces];
    }
    return nSpaces;
}

size_t countFrontSpaces(const std::string &str, const std::string &sep, size_t pos) {
    std::string spaces = " \t";
    size_t strSize = str.size();
    int nSpaces = 0;
    char c = str[pos];

    while (spaces.find(c) != (size_t)-1 && sep.find(c) == (size_t)-1 && c != '\"') {
        nSpaces++;
        if (pos + nSpaces == strSize) {
            break;
        }
        c = str[pos + nSpaces];
    }
    return nSpaces;
}

size_t findEnd(const std::string &str, const std::string &sep, size_t pos) {
    pos = str.find_first_of(sep + "'", pos);
    if (pos == (size_t)-1) {
        return pos;
    }

    if (sep.find(str[pos]) != (size_t)-1) {
        return pos;
    } else {
        size_t endPos = str.find_first_of('\"', pos + 1);
        if (endPos == (size_t)-1) {
            return endPos;
        }
        return findEnd(str, sep, endPos + 1);
    }
}

std::vector<std::string> split(const std::string &str, const std::string &sep) {
    std::vector<std::string> split;
    size_t strLen = str.length();
    size_t start = 0;
    size_t end = -1;

    while (end < strLen || end == (size_t)-1) {
        start = end + 1;
        end = findEnd(str, sep, start);
        end = (end != (size_t)-1) ? end : strLen;

        start += countFrontSpaces(str, sep, start);
        size_t backSpaces = countBackSpaces(str, sep, end);

        size_t size = (end - backSpaces < start) ? 0 : (end - backSpaces - start);
        split.push_back(str.substr(start, size));
    }
    return split;
}

class HttpRequest : public ServerStream {
   private:
    std::string _httpMethod;
    std::string _httpProtocol;
    std::string _url;
    std::unordered_multimap<std::string, std::string> _headers;

    std::string _body;
    size_t _bodySize;

    // char *_body;
    size_t _parsingPos;
    long _currChunkSize;
    size_t _encodedBytesLeftToRead;

    size_t _decodedBodySize;

    size_t _endOfHeadersPos;
    size_t _contentLengthHeader;

    int _isEncodingChunked;
    bool _isComplete;

    size_t parseHead() {
        size_t endOfHead = _rawData.find(CRLF);
        if (endOfHead == (size_t)-1) {
            throw 400;
        }

        const std::vector<std::string> &headElements = split(_rawData.substr(0, endOfHead), " \t");
        if (headElements.size() != 3 ||
            (headElements[1][0] != '/' || headElements[1].find("../") != (size_t)-1) ||
            (headElements[2] != "HTTP/1.0" && headElements[2] != "HTTP/1.1")) {
            throw 400;
        }

        _httpMethod = headElements[0];
        _url = headElements[1];
        _httpProtocol = headElements[2];
        compressSlashes(_url);
        percentDecode(_url);
        return endOfHead + 2;
    }

    size_t parseHeaders() {
        size_t currPos = _parsingPos;
        size_t pos = 0;

        while (currPos < _totalRead) {
            pos = _rawData.find(CRLF, currPos);
            if (pos == (size_t)-1) {
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

    void parseHeaderLine(const std::string &headerLine) {
        int headerLen = headerLine.length();
        int endOfKey = headerLine.find_first_of(':');
        if (endOfKey == -1) {
            return;
        }
        std::string key = headerLine.substr(0, endOfKey);
        endOfKey += 1;

        size_t valueStart = endOfKey + countFrontSpaces(headerLine, "", endOfKey);
        size_t valueEnd = headerLen - countBackSpaces(headerLine, "", headerLen);
        size_t size = (valueEnd < valueStart) ? 0 : (valueEnd - valueStart);
        std::string value = headerLine.substr(valueStart, size);

        _headers.insert(std::make_pair(key, value));
    }

    bool readChunk() {
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

        _currChunkSize = (size_t)-1;
        return false;
    }

    void readChunkSize() {
        // debug("reading chunk size", "", GRAY);
        size_t crlfPos = _rawData.find(CRLF, _parsingPos);
        if (crlfPos == (size_t)-1) {
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

    bool decodeBody() {
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

   public:
    HttpRequest() {
        _endOfHeadersPos = -1;
        _contentLengthHeader = 0;
        _isEncodingChunked = -1;
        _bodySize = 0;
        _isComplete = false;
        _currChunkSize = -1;
        _encodedBytesLeftToRead = 0;
        _decodedBodySize = 0;
    };

    const std::string &getHttpMethod() {
        return _httpMethod;
    }

    const std::string &getProtocol() {
        return _httpProtocol;
    }

    const std::string &getUrl() {
        return _url;
    }

    bool hasBody() {
        return _endOfHeadersPos < _totalRead;
    }

    bool isComplete() {
        return _isComplete;
    }

    const std::string &getBody() {
        return _body;
    }

    bool resumeParsing() {
        try {
            if (_parsingPos == 0) {
                debug("parsing headline", "", GRAY);
                _parsingPos = this->parseHead();
            }

            if (_endOfHeadersPos == (size_t)-1 && _parsingPos < _totalRead) {
                debug("parsing headers", "", GRAY);
                _parsingPos = this->parseHeaders();
            }

            if (_parsingPos >= _endOfHeadersPos) {
                debug("parsing body", "", GRAY);
                _bodySize = _totalRead - _endOfHeadersPos;

                if (_isEncodingChunked) {
                    _isComplete = this->decodeBody();
                } else if (_contentLengthHeader > 0) {
                    if (_bodySize >= _contentLengthHeader) {
                        _body = _rawData.substr(_endOfHeadersPos, _endOfHeadersPos + _contentLengthHeader);
                        _isComplete = true;
                    }
                } else {
                    _isComplete = true;
                }
            }
        } catch (int status) {
            _isComplete = true;
            throw status;
        }
        return _isComplete;
    }

    std::string getHeader(const std::string &key) {
        std::unordered_map<std::string, std::string>::iterator header = _headers.find(key);
        if (header == _headers.end()) {
            return "";
        }
        return header->second;
    }

    std::string findBodyHeader(std::string key, size_t from, size_t to) {
        key = CRLF + key + ":";
        size_t start = _body.find(key, from);
        if (start == (size_t)-1 || start > to) {
            return "";
        }
        start += key.length();

        size_t end = _body.find(CRLF, start);
        if (end == (size_t)-1) {
            return "";
        }

        start += countFrontSpaces(_body, "", start);
        end -= countBackSpaces(_body, "", end);
        return _body.substr(start, end - start);
    }

    std::string findCookie(std::string cookieName) {
        cookieName += "=";
        std::string cookieField = this->getHeader("Cookie");
        if (cookieField == "") {
            return "";
        }

        std::vector<std::string> cookies = split(cookieField, ";");
        for (size_t i = 0; i < cookies.size(); i++) {
            if (cookies[i].compare(0, cookieName.length(), cookieName) == 0) {
                return cookies[i].substr(cookieName.length());
            }
        }
        return "";
    }

    size_t getBodySize() {
        return _bodySize;
    }

    int sendBody(int fd) {
        return this->sendAll(fd, &_body[0], _bodySize);
    }

    size_t searchBody(const std::string &find, size_t from = 0, size_t upto = -1) {
        size_t pos = _body.find(find, from);
        if (upto != (size_t)-1 && pos != (size_t)-1) {
            return (pos <= upto) ? pos : -1;
        }
        return pos;
    }

    // int find(std::string find, size_t pos = 0) {
    //     return _rawData.find(find, pos);
    // }

    std::string subStr(size_t pos, size_t len) {
        return _rawData.substr(pos, len);
    }
};

// 4␍␊
// Wiki␍␊
// 7␍␊
// pedia i␍␊
// B␍␊
// n ␍␊chunks.␍␊
// 0␍␊
// ␍␊

#endif