#ifndef HEADERS_HPP
#define HEADERS_HPP

#include "../webserv.hpp"

class Headers {
   private:
    std::vector<header> _headers;
    String null;

   public:
    Headers();
    Headers(const Headers &h);
    Headers &operator=(const Headers &h);
    void add(const String &field, const String &value);
    void erase(const String &field);
    void add(Headers &headers);
    const String &find(const String &field);
    void putIn(String &buf, String headersToDiscard[] = NULL);
    void clear();
};

#endif