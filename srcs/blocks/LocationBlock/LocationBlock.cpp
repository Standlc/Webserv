#include "../Block.hpp"

class LocationBlock;

LocationBlock::LocationBlock(ServerBlock &serverBlock) : Block(serverBlock),
                                                         _serverBlock(serverBlock) {
    _isExact = false;
    _serverMethodshandlers["GET"] = &LocationBlock::getMethod;
    _serverMethodshandlers["POST"] = &LocationBlock::postMethod;
    _serverMethodshandlers["DELETE"] = &LocationBlock::deleteMethod;
    _requestHandler = &LocationBlock::serverMethodHandler;
    _proxyPass = NULL;
};

LocationBlock::~LocationBlock() {
    delete _proxyPass;
}

LocationBlock::LocationBlock(const LocationBlock &b) : _serverBlock(b._serverBlock) {
    *this = b;
}

LocationBlock &LocationBlock::operator=(const LocationBlock &b) {
    Block::operator=(b);
    _serverMethodshandlers = b._serverMethodshandlers;
    _requestHandler = b._requestHandler;
    _allowedMethods = b._allowedMethods;
    _path = b._path;
    _isExact = b._isExact;
    if (b._proxyPass) {
        _proxyPass = new ProxyUrl(*b._proxyPass);
    } else {
        _proxyPass = NULL;
    }
    _serverBlock = b._serverBlock;
    _redirection = b._redirection;
    _fallBack = b._fallBack;
    return *this;
}

clientPollHandlerType LocationBlock::execute(ClientPoll &client) {
    HttpResponse &res = client.res();
    HttpRequest &req = client.req();

    try {
        throwIf(!isMethodAllowed(req.getHttpMethod()), 405);
        throwIf(exceedsReqMaxSize(req.getBodySize()), 413);

        res.addConfigHeaders(_headers);
        this->handleSessionCookies(client);

        return (this->*_requestHandler)(client);
    } catch (int status) {
        this->loadErrPage(status, res, req);
        return sendResponseToClient;
    }
}

bool isEmpty(const String &str) {
    return str != "" && str != "\"\"";
}

bool isCookieSet(std::vector<String> &cookies, const String &cookie) {
    for (size_t i = 0; i < cookies.size(); i++) {
        std::vector<String> cookieSplit = split(cookies[i], "=");
        if (cookieSplit.size() != 2) {
            continue;
        }
        if (cookieSplit[0] == cookie) {
            return isEmpty(cookieSplit[1]);
        }
    }
    return false;
}

void LocationBlock::handleSessionCookies(ClientPoll &client) {
    HttpResponse &res = client.res();
    HttpRequest &req = client.req();
    const String &cookiesHeader = req.getHeader("Cookie");
    std::vector<String> cookiesSplit = split(cookiesHeader, ";");

    for (size_t i = 0; i < _sessionCookies.size(); i++) {
        const String &cookie = _sessionCookies[i];
        if (cookiesHeader == "" || !isCookieSet(cookiesSplit, cookie)) {
            String cookieValue = cookie + "=" + this->generateSessionCookie();
            res.addConfigHeader("Set-Cookie", cookieValue);
        }
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
