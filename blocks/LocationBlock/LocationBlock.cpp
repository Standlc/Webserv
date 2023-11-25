#include "../Block.hpp"

class LocationBlock;

LocationBlock::LocationBlock(ServerBlock &serverBlock) : Block(serverBlock),
                                                         _serverBlock(serverBlock) {
    _isExact = false;
    _serverMethodshandlers["GET"] = &LocationBlock::getMethod;
    _serverMethodshandlers["POST"] = &LocationBlock::postMethod;
    _serverMethodshandlers["DELETE"] = &LocationBlock::deleteMethod;
    _requestHandler = &LocationBlock::serverMethodHandler;
};

LocationBlock &LocationBlock::operator=(const LocationBlock &b) {
    Block::operator=(b);
    _path = b._path;
    _isExact = b._isExact;
    _serverMethodshandlers = b._serverMethodshandlers;
    return *this;
}

clientPollHandlerType LocationBlock::execute(Server &server, ClientPoll &client) {
    HttpResponse &res = client.res();
    HttpRequest &req = client.req();

    try {
        if (!this->isMethodAllowed(req.getHttpMethod())) {
            throw 405;
        }

        res.addHeaders(_headers);
        if (_sessionCookieName != "" && req.findCookie(_sessionCookieName) == "") {
            res.addHeader("Set-Cookie", _sessionCookieName + "=" + this->generateSessionCookie());
        }

        return (this->*_requestHandler)(client);
    } catch (int status) {
        this->loadErrPage(status, res, req);
    }
    return sendResponseToClient;
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

String LocationBlock::generateSessionCookie() {
    String chars = "0123456789abcdef";
    String cookie;
    for (int i = 0; i < 6; i++) {
        cookie += chars[(int)(std::rand() / (float)RAND_MAX * 15)];
    }
    return cookie;
}
