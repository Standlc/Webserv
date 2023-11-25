#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include "Headers.hpp"
#include "HttpParser.hpp"

class HttpRequest : public HttpParser {
   private:
    int _clientSocket;

   public:
    HttpRequest(int clientSocket);
    bool resumeParsing();

    String findBodyHeader(String key, size_t from, size_t to);
    String findCookie(String cookieName);
    size_t searchBody(const String &find, size_t from = 0, size_t upto = -1);
    String getSocketIpAddress();
    String getClientIpAddress();
    String getClientHostName();
    String getSocketPort();
    void setUrl(const String &url);
    void putHeaders(String &buf, char *headersToDiscard[]);
    void putHead(String &buf);
};

#endif