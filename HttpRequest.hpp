#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include "MediaTypes.hpp"
#include "ServerStream.hpp"
#include "webserv.hpp"

typedef struct URL {
    String path;
    String params;
} URL;

class HttpRequest : public ServerStream {
   private:
    std::unordered_multimap<String, String> _headers;
    int _clientSocket;
    String _httpMethod;
    String _httpProtocol;
    URL _url;

    String _body;
    size_t _bodySize;

    size_t _parsingPos;
    long _currChunkSize;
    size_t _encodedBytesLeftToRead;
    size_t _decodedBodySize;

    size_t _endOfHeadersPos;
    size_t _contentLengthHeader;
    int _isEncodingChunked;
    bool _isComplete;

    size_t parseHead();
    void processUrl(const String &url);
    size_t parseHeaders();
    void parseHeaderLine(const String &headerLine);
    bool readChunk();
    void readChunkSize();
    bool decodeBody();

   public:
    HttpRequest(int clientSocket);
    bool resumeParsing();
    const String &getHttpMethod();
    const String &getProtocol();
    const URL &url();
    bool hasBody();
    bool isComplete();
    const String &getBody();
    String getHeader(const String &key);
    String findBodyHeader(String key, size_t from, size_t to);
    String findCookie(String cookieName);
    size_t getBodySize();
    int sendBody(int fd);
    size_t searchBody(const String &find, size_t from = 0, size_t upto = -1);
    String getSocketIpAddress();
    String getSocketPort();
};

#endif