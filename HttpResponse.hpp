#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include "MediaTypes.hpp"
#include "ServerStream.hpp"
#include "StatusComments.hpp"
#include "webserv.hpp"

class HttpResponse : public ServerStream {
   private:
    int _statusCode;
    std::string _head;
    std::string _defaultHeaders;
    std::unordered_multimap<std::string, std::string> _headers;
    std::string _body;

    std::string _outputData;
    size_t _outputDataSize;

    void setHead(int statusCode) {
        _statusCode = statusCode;
        _head = std::string(HTTP_VERSION) + " ";
        _head += std::to_string(statusCode) + " ";
        _head += StatusComments::get(statusCode) + CRLF;
    }

    void assembleHeaders() {
        _outputData = _head + _defaultHeaders;

        for (std::unordered_multimap<std::string, std::string>::iterator h = _headers.begin(); h != _headers.end(); ++h) {
            _outputData += h->first + ": " + h->second + CRLF;
        }
    }

    void assembleCgiResponse() {
        this->assembleHeaders();
        _outputDataSize = _outputData.size();
        _outputData += _rawData;
        _outputDataSize += _totalRead;
    }

    void assembleResponse() {
        this->assembleHeaders();
        _outputData += CRLF;
        _outputData += _body;
        _outputDataSize = _outputData.size();
    }

   public:
    HttpResponse() : _statusCode(200), _outputDataSize(0) {}

    void clearHeaders() {
        _headers.clear();
    }

    void addHeader(const std::string &property, const std::string &value) {
        _headers.insert(std::make_pair(property, value));
    }

    void addHeaders(std::unordered_multimap<std::string, std::string> &headers) {
        _headers.insert(headers.begin(), headers.end());
    }

    void set(int statusCode) {
        this->setHead(statusCode);
        _defaultHeaders = "";
        _body = "";
        this->assembleResponse();
    }

    void set(int statusCode, const std::string &path, std::string *body) {
        this->setHead(statusCode);
        _defaultHeaders = "Content-Type: " + MediaTypes::getType(path) + CRLF;
        _defaultHeaders += "Content-Length: " + std::to_string(body->size()) + CRLF;
        _body = *body;
        this->assembleResponse();
    }

    void loadFile(int serverStatusCode, const std::string &path) {
        if (path.back() == '/') {
            throw 403;
        }

        int accessStatus = checkPathAccess(path);
        if (accessStatus != 200) {
            throw accessStatus;
        }

        std::string fileContent;
        if (getFileContent(path, fileContent)) {
            debugErr("Error while reading", &path[0]);
            throw 500;
        }
        this->set(serverStatusCode, path, &fileContent);
    }

    void listDirectory(const std::string &dir, const std::string &reqUrl) {
        DIR *dirStream = opendir(&dir[0]);
        if (!dirStream) {
            debugErr("opendir", strerror(errno));
            throw 500;
        }

        struct dirent *entry;
        readNextEntry(dirStream, &entry);

        std::string listingPage = generateDirectoryListingPage(dir, reqUrl, entry, dirStream);
        closedir(dirStream);
        this->set(200, ".html", &listingPage);
    }

    void setCgiResponse(int statusCode) {
        this->setHead(statusCode);
        this->assembleCgiResponse();
    }

    int sendResponse(int fd) {
        return this->sendAll(fd, &_outputData[0], _outputDataSize);
    }

    std::string &outputData() {
        return _outputData;
    }

    int status() {
        return _statusCode;
    }
};

#endif