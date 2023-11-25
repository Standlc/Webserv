#ifndef HEADERS_HPP
#define HEADERS_HPP

#include "../webserv.hpp"

class Headers {
   private:
    std::vector<header> _headers;
    String null;

   public:
    void add(const String &field, const String &value) {
        _headers.push_back((header){field, value});
    }

    void erase(const String &field) {
        for (size_t i = 0; i < _headers.size(); i++) {
            if (_headers[i].field == field) {
                _headers.erase(_headers.begin() + i);
            }
        }
    }

    void add(Headers &headers) {
        _headers.insert(_headers.end(), headers._headers.begin(), headers._headers.end());
    }

    const String &find(const String &field) {
        for (size_t i = 0; i < _headers.size(); i++) {
            if (_headers[i].field == field) {
                return _headers[i].value;
            }
        }
        return null;
    }

    void putIn(String &buf, char *headersToDiscard[] = NULL) {
        for (size_t i = 0; i < _headers.size(); i++) {
            if (headersToDiscard) {
                bool discard = false;
                for (size_t j = 0; headersToDiscard[j]; j++) {
                    if ((discard = _headers[i].field == headersToDiscard[j])) {
                        break;
                    }
                }
                if (discard) {
                    continue;
                }
            }

            buf += _headers[i].field + ": " + _headers[i].value + CRLF;
        }
    }

    void clear() {
        _headers.clear();
    }
};

#endif