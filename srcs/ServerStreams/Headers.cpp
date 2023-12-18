#include "Headers.hpp"

Headers::Headers() {
    null = "";
};

Headers::Headers(const Headers &h) {
    *this = h;
}

Headers &Headers::operator=(const Headers &h) {
    _headers = h._headers;
    null = "";
    return *this;
}

void Headers::add(const String &field, const String &value) {
    _headers.push_back((header){lowercase(field), value});
}

void Headers::erase(const String &field) {
    String lowercaseField = lowercase(field);

    for (size_t i = 0; i < _headers.size(); i++) {
        if (_headers[i].field == lowercaseField) {
            _headers.erase(_headers.begin() + i);
        }
    }
}

void Headers::add(Headers &headers) {
    _headers.insert(_headers.end(), headers._headers.begin(), headers._headers.end());
}

const String &Headers::find(const String &field) {
    String lowercaseField = lowercase(field);

    for (size_t i = 0; i < _headers.size(); i++) {
        if (_headers[i].field == lowercaseField) {
            return _headers[i].value;
        }
    }
    return null;
}

void Headers::putIn(String &buf, String headersToDiscard[]) {
    if (headersToDiscard) {
        for (size_t i = 0; headersToDiscard[i] != ""; i++) {
            headersToDiscard[i] = lowercase(headersToDiscard[i]);
        }
    }

    for (size_t i = 0; i < _headers.size(); i++) {
        if (headersToDiscard) {
            bool discard = false;
            for (size_t j = 0; headersToDiscard[j] != ""; j++) {
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

void Headers::clear() {
    _headers.clear();
}