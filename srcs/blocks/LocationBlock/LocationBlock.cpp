#include "../Block.hpp"

class LocationBlock;

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
        this->loadErrPage(status, res, _path);
        return sendResponseToClient;
    }
}

clientPollHandlerType LocationBlock::serverMethodHandler(ClientPoll &client) {
    HttpResponse &res = client.res();
    HttpRequest &req = client.req();
    const String &reqHttpMethod = req.getHttpMethod();

    throwIf(!handlesHttpMethod(reqHttpMethod), 501);

    String cgiScriptPath = this->isCgiScriptRequest(req);
    if (cgiScriptPath != "") {
        return this->handleCgi(client, cgiScriptPath);
    }

    (this->*(_serverMethodshandlers[reqHttpMethod]))(req, res);
    return sendResponseToClient;
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
