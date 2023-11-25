#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include "../MediaTypes.hpp"
#include "../StatusComments.hpp"
#include "../webserv.hpp"
#include "Headers.hpp"
#include "HttpRequest.hpp"
#include "ServerStream.hpp"

class HttpResponse : public ServerStream {
   private:
    int _statusCode;
    String _head;
    Headers _defaultHeaders;
    Headers _headers;
    String _body;
    HttpRequest &_req;

    String _outputData;
    size_t _outputDataSize;
    bool _keepAlive;

    unorderedStringMap _cgiHeaders;

    void setHead(int statusCode);
    void assembleHeaders();
    void assembleResponse();
    void setConnectionType(int statusCode);

   public:
    HttpResponse(HttpRequest &req);
    ~HttpResponse();

    void clearHeaders();
    void addHeader(const String &field, const String &value);
    void addDefaultHeader(const String &field, const String &value);
    void addHeaders(Headers &headers);
    void set(int statusCode);
    void set(int statusCode, const String &path, const String &body);
    void setBody(const String &body);
    void loadFile(int serverStatusCode, const String &path);
    void listDirectory(const String &dir, const String &reqUrl);
    int sendResponse(int fd);
    const String &outputData();
    int status();
    bool keepAlive();
};

#endif