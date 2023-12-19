#ifndef CGI_RESPONSE_HPP
#define CGI_RESPONSE_HPP

#include "../HttpParser.hpp"
#include "../HttpResponse.hpp"

class CgiResponse : public HttpParser {
   private:
    HttpResponse &_clientRes;
    int _statusHeaderCode;
    String location;

   public:
    CgiResponse(HttpResponse &clientRes);
    void parseStatusHeader();
    void setClientResponse();
    bool resumeParsing();
    const String &getHeader(const String &field);
};

#endif