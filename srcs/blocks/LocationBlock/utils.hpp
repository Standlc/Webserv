#ifndef LOCATION_BLOCK_UTILS_HPP
#define LOCATION_BLOCK_UTILS_HPP

#include "../../webserv.hpp"

class ProxyUrl {
   private:
    String _fullUrl;
    String _host;
    String _port;
    String _path;

   public:
    ProxyUrl(const String &proxyUrl) {
        _fullUrl = proxyUrl;
        _host = this->parseHost();
        _port = this->parsePort();
        _path = this->parsePath();
        // debug("host", _host);
        // debug("port", _port);
        // debug("path", _path);
    }

    ProxyUrl(const ProxyUrl &other) {
        _fullUrl = other._fullUrl;
        _host = other._host;
        _port = other._port;
        _path = other._path;
    }

    String parseHost() {
        size_t start = String("http://").size();
        size_t end = _fullUrl.find(':', start);
        size_t hostLen = 0;

        if (end == NPOS) {
            end = _fullUrl.find_first_of("/?", start);
        }

        hostLen = (end == NPOS) ? -1 : end - start;
        return _fullUrl.substr(start, hostLen);
    }

    String parsePort() {
        size_t portPos = _fullUrl.find(':', String("http://").size());
        if (portPos == NPOS) {
            return "80";
        }

        portPos += 1;
        size_t portEndPos = _fullUrl.find_first_of("/?", portPos);
        size_t portLen = (portEndPos == NPOS) ? -1 : portEndPos - portPos;
        return _fullUrl.substr(portPos, portLen);
    }

    String parsePath() {
        size_t offset = String("http://").size();
        size_t start = _fullUrl.find_first_of("/?", offset);
        if (start == NPOS) {
            return "/";
        }
        return _fullUrl.substr(start, -1);
    }

    const String &host() {
        return _host;
    }
    const String &port() {
        return _port;
    }
    const String &path() {
        return _path;
    }
};

#endif