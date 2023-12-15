#include "../Block.hpp"

clientPollHandlerType LocationBlock::redirectionHandler(ClientPoll &client) {
    HttpResponse &res = client.res();
    HttpRequest &req = client.req();

    res.addConfigHeader("Location", this->assembleRedirectionUrl(req));
    throw _redirection.statusCode;
}

String LocationBlock::assembleRedirectionUrl(HttpRequest &req) {
    if (startsWith(_redirection.url, "http:") || startsWith(_redirection.url, "https:")) {
        return _redirection.url;
    }
    if (_redirection.url[0] == '/') {
        return "http://" + req.getHeader("Host") + _redirection.url;
    }
    return _redirection.url;
}