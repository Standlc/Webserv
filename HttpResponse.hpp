#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include "MediaTypes.hpp"
#include "ServerStream.hpp"
#include "StatusComments.hpp"
#include "webserv.hpp"

class HttpResponse : public ServerStream {
   private:
    int _statusCode;
    String _head;
    String _defaultHeaders;
    unorderedStringMultiMap _headers;
    String _body;

    String _outputData;
    size_t _outputDataSize;

    void setHead(int statusCode);
    void assembleHeaders();
    void assembleCgiResponse();
    void assembleResponse();

   public:
    HttpResponse();

    void clearHeaders();
    void addHeader(const String &property, const String &value);
    void addHeaders(unorderedStringMultiMap &headers);
    void set(int statusCode);
    void set(int statusCode, const String &path, String *body);
    void loadFile(int serverStatusCode, const String &path);
    void listDirectory(const String &dir, const String &reqUrl);
    void setCgiResponse(int statusCode);
    int sendResponse(int fd);
    const String &outputData();
    String headers();
    int status();
};

#endif