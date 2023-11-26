#ifndef HTTP_MESSAGE_HPP
#define HTTP_MESSAGE_HPP

#include "Headers.hpp"
#include "ServerStream.hpp"

typedef struct URL {
    String path;
    String params;
} URL;

class HttpParser : public ServerStream {
   protected:
    String _httpMethod;
    int _responseStatus;
    String _rawUrl;
    URL _url;
    String _httpProtocol;
    Headers _headers;
    String _body;

    size_t _parsingPos;
    size_t _headerLineEndPos;
    size_t _endOfHeadersPos;
    long _contentLengthHeader;
    bool _isEncodingChunked;
    size_t _bodySize;
    bool _isComplete;
    long _currChunkSize;
    long _encodedBytesLeftToRead;

    bool resumeParsing(bool decodeBody = false);
    bool discardEmptyLine(int *sizeCRLF);
    bool parseRequestHead();
    bool parseResponseHead();
    void processUrl(const String &url);
    size_t parseHeaders();
    void getBodyInfos();
    void parseHeaderLine(const String &headerLine);
    bool handleBodyParsing(bool copyBody);
    bool resumeDecodingBody(bool copyBody);
    bool readChunk(bool copyBody);
    bool readChunkSize();
    void appendBody(bool copyBody, size_t from, size_t size);
    size_t findCRLF(int *sizeCRLF, size_t from = 0);

   public:
    HttpParser() {
        _endOfHeadersPos = -1;
        _headerLineEndPos = -1;
        _contentLengthHeader = -1;
        _isEncodingChunked = false;
        _bodySize = 0;
        _isComplete = false;
        _currChunkSize = -1;
        _encodedBytesLeftToRead = 0;
        _parsingPos = 0;
    }

    const String &getHttpMethod() {
        return _httpMethod;
    }
    const String &getProtocol() {
        return _httpProtocol;
    }
    const URL &url() {
        return _url;
    }
    const String &rawUrl() {
        return _rawUrl;
    }
    bool hasBody() {
        return _bodySize > 0;
    }
    bool isComplete() {
        return _isComplete;
    }
    const String &getBody() {
        return _body;
    }
    String getHeader(const String &key) {
        return _headers.find(key);
    }
    size_t getBodySize() {
        return _bodySize;
    }
};

#endif