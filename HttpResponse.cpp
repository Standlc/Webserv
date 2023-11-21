#include "HttpResponse.hpp"

void HttpResponse::setHead(int statusCode) {
    _statusCode = statusCode;
    _head = String(HTTP_VERSION) + " ";
    _head += std::to_string(statusCode) + " ";
    _head += StatusComments::get(statusCode) + CRLF;
}

void HttpResponse::assembleHeaders() {
    _outputData = _head + _defaultHeaders;

    for (unorderedStringMultiMap::iterator h = _headers.begin(); h != _headers.end(); ++h) {
        _outputData += h->first + ": " + h->second + CRLF;
    }
}

void HttpResponse::assembleCgiResponse() {
    this->assembleHeaders();
    _outputDataSize = _outputData.size();
    _outputData += _rawData;
    _outputDataSize += _totalRead;
}

void HttpResponse::assembleResponse() {
    this->assembleHeaders();
    _outputData += CRLF;
    _outputData += _body + CRLF;
    _outputDataSize = _outputData.size();
}

HttpResponse::HttpResponse() : _statusCode(200), _outputDataSize(0) {}

void HttpResponse::clearHeaders() {
    _headers.clear();
}

void HttpResponse::addHeader(const String &property, const String &value) {
    _headers.insert(std::make_pair(property, value));
}

void HttpResponse::addHeaders(unorderedStringMultiMap &headers) {
    _headers.insert(headers.begin(), headers.end());
}

void HttpResponse::set(int statusCode) {
    this->setHead(statusCode);
    _defaultHeaders = "";
    _body = "";
    this->assembleResponse();
}

void HttpResponse::set(int statusCode, const String &path, String *body) {
    this->setHead(statusCode);
    _defaultHeaders = "Content-Type: " + MediaTypes::getType(path) + CRLF;
    _defaultHeaders += "Content-Length: " + std::to_string(body->size()) + CRLF;
    _defaultHeaders += "Server: " + String(WEBSERV_V) + String(CRLF);
    if (statusCode >= 400) {
        _defaultHeaders += "Connection: close" + String(CRLF);
    } else {
        _defaultHeaders += "Connection: keep-alive" + String(CRLF);
    }

    _body = *body;
    this->assembleResponse();
}

void HttpResponse::loadFile(int serverStatusCode, const String &path) {
    if (path.back() == '/') {
        throw 403;
    }

    int accessStatus = checkPathAccess(path);
    if (accessStatus != 200) {
        throw accessStatus;
    }

    String fileContent;
    if (getFileContent(path, fileContent)) {
        debugErr("Error while reading", &path[0]);
        throw 500;
    }
    this->set(serverStatusCode, path, &fileContent);
}

void HttpResponse::listDirectory(const String &dir, const String &reqUrl) {
    DIR *dirStream = opendir(&dir[0]);
    if (!dirStream) {
        debugErr("opendir", strerror(errno));
        throw 500;
    }

    struct dirent *entry;
    readNextEntry(dirStream, &entry);

    String listingPage = generateDirectoryListingPage(dir, reqUrl, entry, dirStream);
    closedir(dirStream);
    this->set(200, ".html", &listingPage);
}

void HttpResponse::setCgiResponse(int statusCode) {
    this->setHead(statusCode);
    this->assembleCgiResponse();
}

int HttpResponse::sendResponse(int fd) {
    return this->sendAll(fd, &_outputData[0], _outputDataSize);
}

const String &HttpResponse::outputData() {
    return _outputData;
}

String HttpResponse::headers() {
    String res = _head + _defaultHeaders;

    for (unorderedStringMultiMap::iterator h = _headers.begin(); h != _headers.end(); ++h) {
        res += h->first + ": " + h->second + CRLF;
    }
    return res;
}

int HttpResponse::status() {
    return _statusCode;
}