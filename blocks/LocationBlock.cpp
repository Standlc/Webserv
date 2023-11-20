#include "Block.hpp"

class LocationBlock;

LocationBlock::LocationBlock(const Block &b) : Block(b), _isExact(false){};

clientPollHandlerType LocationBlock::execute(Server &server, ClientPoll &client) {
    HttpResponse &res = client.res();
    HttpRequest &req = client.req();
    String cgiScriptPath;

    try {
        res.addHeaders(_headers);

        if (_redirection.url != "") {
            res.addHeader("Location", this->assembleRedirectionUrl(req));
            throw _redirection.statusCode;
        }

        // proxyPass()
        this->throwReqErrors(req);

        if ((cgiScriptPath = this->isCgiScriptRequest(req)) != "") {
            return handleCgi(server, client, cgiScriptPath);
        }

        _handlers[req.getHttpMethod()](*this, req, res);

        if (_sessionCookieName != "" && req.findCookie(_sessionCookieName) == "") {
            res.addHeader("Set-Cookie", _sessionCookieName + "=" + this->generateSessionCookie());
        }
    } catch (int status) {
        if (status >= 400) {
            res.clearHeaders();
        }
        this->loadErrPage(status, res, req);
    }
    return sendResponseToClient;
}

LocationBlock &LocationBlock::operator=(const LocationBlock &b) {
    Block::operator=(b);
    _path = b._path;
    _isExact = b._isExact;
    _handlers = b._handlers;
    return *this;
}

void LocationBlock::handlePOSTCgi(Server &server, ClientPoll &client, const String &cgiResourcePath, const String &cgiScriptCommand) {
    int pipes[2];
    tryPipe(pipes);
    client.execveCgi(server, cgiResourcePath, cgiScriptCommand, pipes);
    server.pushNewCgiRequestPoll(pipes, client);
}

clientPollHandlerType LocationBlock::handleCgi(Server &server, ClientPoll &client, const String &cgiScriptPath) {
    debug("cgi script path", cgiScriptPath, YELLOW);
    String cgiResourcePath = this->getResourcePath(cgiScriptPath);
    debug("cgi resource path", cgiResourcePath, YELLOW);
    this->checkCgiScriptAccess(cgiScriptPath);

    this->setenvCgi(client.req(), cgiScriptPath);
    String cgiExtension = getFileExtension(cgiScriptPath);
    if (client.req().getHttpMethod() == "POST" && client.req().hasBody()) {
        this->handlePOSTCgi(server, client, cgiResourcePath, _cgiCommands[cgiExtension]);
        return checkCgiWrite;
    }
    client.execveCgi(server, cgiResourcePath, _cgiCommands[cgiExtension]);
    return waitCgiProcessEnd;
}

void LocationBlock::setenvCgi(HttpRequest &req, const String &cgiScriptPath) {
    trySetenv("REQUEST_METHOD", req.getHttpMethod());
    trySetenv("SERVER_SOFTWARE", "webserv/1.0");
    trySetenv("GATEWAY_INTERFACE", "CGI/1.1");
    trySetenv("SERVER_NAME", &(split(req.getHeader("Host"), ":")[0])[0]);
    trySetenv("SERVER_PORT", req.getSocketPort());
    trySetenv("SERVER_PROTOCOL", req.getProtocol());
    trySetenv("REMOTE_ADDR", &req.getSocketIpAddress()[0]);
    // trySetenv("REMOTE_HOST", &req.getSocketIpAddress()[0]);

    trySetenv("HTTP_ACCEPT", req.getHeader("Accept"));
    trySetenv("HTTP_USER_AGENT", req.getHeader("User-Agent"));
    trySetenv("HTTP_ACCEPT_LANGUAGE", req.getHeader("Accept-Language"));
    trySetenv("HTTP_COOKIE", req.getHeader("Cookie"));
    trySetenv("HTTP_REFERER", req.getHeader("Referer"));

    trySetenv("SCRIPT_NAME", cgiScriptPath);
    const String &reqUrl = req.url().path;
    const String &pathInfo = reqUrl.substr(cgiScriptPath.size(), -1);
    trySetenv("PATH_INFO", pathInfo);
    trySetenv("PATH_TRANSLATED", this->getResourcePath(pathInfo));
    // trySetenv("SCRIPT_FILENAME", cgiPaths.);

    if (req.getHttpMethod() == "GET") {
        trySetenv("QUERY_STRING", req.url().params);
        trySetenv("CONTENT_LENGTH", "");
        trySetenv("CONTENT_TYPE", "");
    }

    if (req.hasBody()) {
        trySetenv("QUERY_STRING", "");
        trySetenv("CONTENT_LENGTH", req.getHeader("Content-Length"));
        trySetenv("CONTENT_TYPE", req.getHeader("Content-Type"));
    }
}

void LocationBlock::checkCgiScriptAccess(const String &cgiScriptPath) {
    String resourcePath = this->getResourcePath(cgiScriptPath);

    int fileAccessStatus = checkPathAccess(resourcePath);
    if (fileAccessStatus != 200) {
        throw fileAccessStatus;
    }
    if (isDirectory(resourcePath)) {
        throw 400;
    }
}

String LocationBlock::isCgiScriptRequest(HttpRequest &req) {
    const String &reqUrl = req.url().path;

    for (unorderedStringMap::iterator i = _cgiCommands.begin(); i != _cgiCommands.end(); ++i) {
        size_t pos = reqUrl.find(i->first);

        while (pos != NPOS) {
            char reqCharAfterExt = reqUrl[pos + i->first.size()];
            if (reqCharAfterExt == '/' || reqCharAfterExt == '\0') {
                return reqUrl.substr(0, pos + i->first.size());
            }
            pos = reqUrl.find(i->first, pos + 1);
        }
    }
    return "";
}

// void proxyPass(Server &server, ClientPoll &client) {
//     // Change host name
//     // change connection to close
//     // Send 502 if request fails or res status is error
// }

void LocationBlock::throwReqErrors(HttpRequest &req) {
    String reqHttpMethod = req.getHttpMethod();
    if (isUnkownMethod(reqHttpMethod)) {
        throw 501;
    }
    if (!this->handlesHttpMethod(reqHttpMethod)) {
        throw 405;
    }
    if (reqHttpMethod == "POST") {
        if (this->exceedsReqMaxSize(req.getBodySize())) {
            throw 413;
        }
    }
}

String LocationBlock::assembleRedirectionUrl(HttpRequest &req) {
    if (_redirection.url.compare(0, 5, "http:") == 0 || _redirection.url.compare(0, 6, "https:") == 0) {
        return _redirection.url;
    }
    if (_redirection.url[0] == '/') {
        return "http://" + req.getHeader("Host") + _redirection.url;
    }
    return _redirection.url;
}

bool LocationBlock::exceedsReqMaxSize(size_t size) {
    if (_reqBodyMaxSize == NPOS) {
        return false;
    }
    return size > _reqBodyMaxSize;
}

String LocationBlock::generateSessionCookie() {
    String chars = "0123456789abcdef";
    String cookie;
    for (int i = 0; i < 6; i++) {
        cookie += chars[(int)(std::rand() / (float)RAND_MAX * 15)];
    }
    return cookie;
}

void LocationBlock::setPath(const String &path, bool isExact) {
    if (path[0] != '/') {
        _path = "/" + path;
    }
    _path = path;
    compressSlashes(_path);
    _isExact = isExact;
}

void LocationBlock::setHandlers(pathHandlerType getMethod, pathHandlerType postMethod, pathHandlerType deleteMethod) {
    if (getMethod != NULL) {
        _handlers["GET"] = getMethod;
    }
    if (postMethod != NULL) {
        _handlers["POST"] = postMethod;
    }
    if (deleteMethod != NULL) {
        _handlers["DELETE"] = deleteMethod;
    }
}

bool LocationBlock::handlesHttpMethod(String httpMethod) {
    return _handlers.find(httpMethod) != _handlers.end();
}

const String &LocationBlock::getPath() {
    return _path;
}

bool LocationBlock::isExactPath() {
    return _isExact;
}

const String &LocationBlock::getIndex() {
    return _index;
}

bool LocationBlock::isAutoIndex() {
    return _autoIndex;
}