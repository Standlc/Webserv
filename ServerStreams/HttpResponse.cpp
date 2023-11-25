#include "HttpResponse.hpp"

void HttpResponse::setBody(const String &body) {
    _body = body;
}

void HttpResponse::setHead(int statusCode) {
    _statusCode = statusCode;
    _head = String(HTTP_VERSION) + " ";
    _head += std::to_string(statusCode) + " ";
    _head += StatusComments::get(statusCode) + CRLF;
}

void HttpResponse::assembleHeaders() {
    _outputData = _head;
    _defaultHeaders.putIn(_outputData);
    _headers.putIn(_outputData);
}

void HttpResponse::assembleResponse() {
    this->assembleHeaders();
    _outputData += CRLF;
    _outputData += _body;
    _outputDataSize = _outputData.size();
}

HttpResponse::HttpResponse(HttpRequest &req) : _req(req) {
    _statusCode = 200;
    _outputDataSize = 0;
    _keepAlive = false;
}

HttpResponse::~HttpResponse() {
}

void HttpResponse::clearHeaders() {
    _headers.clear();
}

void HttpResponse::addDefaultHeader(const String &field, const String &value) {
    _defaultHeaders.add(field, value);
}

void HttpResponse::addHeader(const String &property, const String &value) {
    _headers.add(property, value);
}

void HttpResponse::addHeaders(Headers &headers) {
    _headers.add(headers);
}

void HttpResponse::set(const String &response) {
    // _outputData = response;
    // _outputDataSize = response.size();
}

void HttpResponse::set(int statusCode) {
    this->setHead(statusCode);
    _defaultHeaders.clear();
    if (statusCode >= 400) {
        this->clearHeaders();
    }
    this->addDefaultHeader("Server", WEBSERV_V);
    this->setConnectionType(statusCode);
    this->assembleResponse();
}

void HttpResponse::set(int statusCode, const String &path, const String &body) {
    this->setHead(statusCode);

    _defaultHeaders.clear();
    if (statusCode >= 400) {
        this->clearHeaders();
    }
    this->addDefaultHeader("Server", WEBSERV_V);
    this->addDefaultHeader("Content-Type", MediaTypes::getType(path));
    this->addDefaultHeader("Content-Length", std::to_string(body.size()));
    this->setConnectionType(statusCode);

    this->setBody(body);
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
    getFileContent(path, fileContent);
    this->set(serverStatusCode, path, fileContent);
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
    this->set(200, ".html", listingPage);
}

int HttpResponse::sendResponse(int fd) {
    debugMessageInfos("sending client response to", fd, _outputDataSize, CYAN);
    debugHttpMessage(_outputData, CYAN);
    return this->sendAll(fd, &_outputData[0], _outputDataSize);
}

void HttpResponse::setConnectionType(int statusCode) {
    if (statusCode >= 400 || _req.getHeader("Connection") == "close") {
        this->addDefaultHeader("Connection", "close");
        _keepAlive = false;
        return;
    }

    const String &connection = _headers.find("Connection");
    if (connection == "close") {
        _keepAlive = false;
    } else if (connection == "") {
        this->addDefaultHeader("Connection", "keep-alive");
        _keepAlive = true;
    }
}

const String &HttpResponse::outputData() {
    return _outputData;
}

int HttpResponse::status() {
    return _statusCode;
}

bool HttpResponse::keepAlive() {
    return _keepAlive;
}