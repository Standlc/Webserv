#include "../../Block.hpp"

clientPollHandlerType LocationBlock::serverMethodHandler(ClientPoll &client) {
    HttpResponse &res = client.res();
    HttpRequest &req = client.req();
    String cgiScriptPath;

    this->throwReqErrors(req);

    if ((cgiScriptPath = this->isCgiScriptRequest(req)) != "") {
        return this->handleCgi(client, cgiScriptPath);
    }
    (this->*(_serverMethodshandlers[req.getHttpMethod()]))(req, res);
    return sendResponseToClient;
}

void LocationBlock::throwReqErrors(HttpRequest &req) {
    String reqHttpMethod = req.getHttpMethod();
    if (isUnkownMethod(reqHttpMethod)) {
        throw 501;
    }
    if (!this->handlesHttpMethod(reqHttpMethod)) {
        throw 405;
    }
    if (this->exceedsReqMaxSize(req.getBodySize())) {
        throw 413;
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

void closeSocketPairs(CgiSockets &sockets) {
    closeOpenFd(sockets.response[0]);
    closeOpenFd(sockets.response[1]);
    closeOpenFd(sockets.request[0]);
    closeOpenFd(sockets.request[1]);
}

void createCgiSocketPair(int *sockets, long socketCapacity) {
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == -1) {
        throw 500;
    }
    if (setsockopt(sockets[0], SOL_SOCKET, SO_SNDBUF, &socketCapacity, sizeof(socketCapacity)) == -1) {
        throw 500;
    }
    if (setsockopt(sockets[1], SOL_SOCKET, SO_SNDBUF, &socketCapacity, sizeof(socketCapacity)) == -1) {
        throw 500;
    }
}

CgiSockets createCgiReqResSocketPairs() {
    CgiSockets sockets = {{-1, -1}, {-1, -1}};
    int cgiSockSize = CGI_SOCK_SIZE;
    try {
        createCgiSocketPair(sockets.response, cgiSockSize);
        createCgiSocketPair(sockets.request, cgiSockSize);
    } catch (int status) {
        closeSocketPairs(sockets);
        throw status;
    }
    return sockets;
}

clientPollHandlerType LocationBlock::handleCgi(ClientPoll &client, const String &cgiScriptPath) {
    String cgiResourcePath = this->getResourcePath(cgiScriptPath);
    // debug("cgi script path", cgiScriptPath, YELLOW);
    // debug("cgi resource path", cgiResourcePath, YELLOW);
    this->checkCgiScriptAccess(cgiScriptPath);
    this->setenvCgi(client.req(), cgiScriptPath);

    String cgiExtension = getFileExtension(cgiScriptPath);
    CgiSockets cgiSockets = createCgiReqResSocketPairs();
    CgiPoll &cgiPoll = client.server().pushNewCgiPoll(cgiSockets, client);

    if (client.req().hasBody()) {
        cgiPoll.switchToRequestWritableSocket();
        cgiPoll.setReadHandler(NULL);
        cgiPoll.setWriteHandler(sendCgiRequest);
    }
    cgiPoll.forkAndexecuteScript(cgiResourcePath, _cgiCommands[cgiExtension]);
    return checkCgiPoll;
}

String parseCgiPathInfo(HttpRequest &req, const String &cgiScriptPath) {
    const String &reqUrl = req.url().path;
    return reqUrl.substr(cgiScriptPath.size(), -1);
}

void LocationBlock::setenvCgi(HttpRequest &req, const String &cgiScriptPath) {
    trySetenv("GATEWAY_INTERFACE", "CGI/1.1");
    trySetenv("SERVER_SOFTWARE", WEBSERV_V);
    trySetenv("SERVER_NAME", &(split(req.getHeader("Host"), ":")[0])[0]);
    trySetenv("SERVER_PORT", req.getSocketPort());
    trySetenv("SERVER_PROTOCOL", req.getProtocol());
    trySetenv("REMOTE_ADDR", &req.getClientIpAddress()[0]);
    trySetenv("REMOTE_HOST", &req.getClientHostName()[0]);

    trySetenv("REQUEST_METHOD", req.getHttpMethod());
    trySetenv("QUERY_STRING", req.url().params);
    trySetenv("SCRIPT_NAME", cgiScriptPath);

    String pathInfo = parseCgiPathInfo(req, cgiScriptPath);
    trySetenv("PATH_INFO", pathInfo);
    if (pathInfo != "") {
        trySetenv("PATH_TRANSLATED", this->getResourcePath(pathInfo));
    } else {
        unsetenv("PATH_TRANSLATED");
    }

    if (req.hasBody()) {
        trySetenv("CONTENT_LENGTH", std::to_string(req.getBodySize()));
        trySetenv("CONTENT_TYPE", req.getHeader("Content-Type"));
    } else {
        tryUnsetenv("CONTENT_LENGTH");
    }

    // trySetenv("REDIRECT_STATUS", "200"); //Security needed to execute php-cgi
    // trySetenv("HTTP_ACCEPT", req.getHeader("Accept"));
    // trySetenv("HTTP_USER_AGENT", req.getHeader("User-Agent"));
    // trySetenv("HTTP_ACCEPT_LANGUAGE", req.getHeader("Accept-Language"));
    // trySetenv("HTTP_COOKIE", req.getHeader("Cookie"));
    // trySetenv("HTTP_REFERER", req.getHeader("Referer"));
}
