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
            _statusHeaderCode = toInt(statusHeader);
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
    _headers.erase("Server");
    _headers.erase("Content-Length");
    _headers.add("Content-Length", toString(_bodySize));
    _clientRes.addDefaultHeaders(_headers);

    _clientRes.setBody(_rawData.substr(_endOfHeadersPos, _bodySize));
    _clientRes.set(_statusHeaderCode);
}

bool CgiResponse::resumeParsing() {
    if (HttpParser::resumeParsing(false) == true) {
        return true;
    }
    return false;
}