#include "CgiResponse.hpp"

CgiResponse::CgiResponse(HttpResponse &clientRes) : _clientRes(clientRes) {
    _statusHeaderCode = 200;
}

const String &CgiResponse::getHeader(const String &field) {
    return _headers.find(field);
}

void CgiResponse::parseStatusHeader() {
    const String &statusHeader = _headers.find("Status");
    if (statusHeader != "") {
        try {
            _statusHeaderCode = std::stoi(statusHeader);
        } catch (const std::exception &e) {
            std::cerr << e.what() << '\n';
            throw 502;
        }
    }
}

void CgiResponse::parseLocationHeader() {
    const String &location = _headers.find("Location");
    if (location == "") {
        return;
    }
    if (!startsWith(location, "http://") && !startsWith(location, "https://") && location[0] != '/') {
        throw 502;
    }
}

void CgiResponse::setClientResponse() {
    _clientRes.addHeaders(_headers);
    _clientRes.setBody(&_rawData[_endOfHeadersPos]);
    _clientRes.set(_statusHeaderCode);
}

bool CgiResponse::resumeParsing() {
    return HttpParser::resumeParsing(false);
}