#include "../Block.hpp"

String LocationBlock::isCgiScriptRequest(HttpRequest &req) {
    const String &reqUrl = req.url().path;

    for (stringMap::iterator i = _cgiCommands.begin(); i != _cgiCommands.end(); ++i) {
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
    throwIf(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == -1, 500);
    throwIf(setsockopt(sockets[0], SOL_SOCKET, SO_SNDBUF, &socketCapacity, sizeof(socketCapacity)) == -1, 500);
    throwIf(setsockopt(sockets[1], SOL_SOCKET, SO_SNDBUF, &socketCapacity, sizeof(socketCapacity)) == -1, 500);
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

String LocationBlock::cgiScriptResourcePath(const String &cgiScriptPath) {
    size_t pathSize = _path.size();
    String reqPathInfo;

    if (pathSize > cgiScriptPath.size()) {
        reqPathInfo = "";
    } else {
        reqPathInfo = cgiScriptPath.substr(pathSize);
    }
    return this->getResourcePath(_path, reqPathInfo[0] == '/' ? &reqPathInfo[1] : reqPathInfo);
}

clientPollHandlerType LocationBlock::handleCgi(ClientPoll &client, const String &cgiScriptPath) {
    String cgiResourcePath = this->cgiScriptResourcePath(cgiScriptPath);
    this->checkCgiScriptAccess(cgiResourcePath);
    this->setenvCgi(client.req(), cgiResourcePath, cgiScriptPath);

    String cgiExtension = getFileExtension(cgiScriptPath);
    CgiSockets cgiSockets = createCgiReqResSocketPairs();
    CgiPoll &cgiPoll = client.server().pushNewCgiPoll(cgiSockets, client);

    if (client.req().hasBody()) {
        cgiPoll.switchToRequestWritableSocket();
        cgiPoll.setReadHandler(NULL);
        cgiPoll.setWriteHandler(sendCgiRequest);
    }
    cgiPoll.forkAndexecuteScript(client.server(), cgiResourcePath, _cgiCommands[cgiExtension]);
    return checkCgiPoll;
}

void LocationBlock::checkCgiScriptAccess(const String &cgiResourcePath) {
    int fileAccessStatus = checkPathAccess(cgiResourcePath);
    throwIf(fileAccessStatus != 200, fileAccessStatus);
    throwIf(isDirectory(cgiResourcePath), 403);
}

String parseCgiPathInfo(HttpRequest &req, const String &cgiScriptPath) {
    const String &reqUrl = req.url().path;
    size_t reqUrlSize = reqUrl.size();
    size_t cgiScriptPathSize = cgiScriptPath.size();
    if (reqUrlSize < cgiScriptPathSize) {
        return "";
    }
    return reqUrl.substr(cgiScriptPathSize, -1);
}

int countSlashes(String path) {
    compressSlashes(path);

    int slashCount = 0;
    size_t pos = 0;
    size_t end = 0;

    while (end != NPOS) {
        pos = path.find_first_of('/', end);
        if (pos == NPOS) {
            break;
        }
        slashCount++;
        end = path.find_last_not_of('/', pos + 1);
    }
    return slashCount;
}

String LocationBlock::getPathTranslated(const String &pathInfo, const String &cgiResourcePath) {
    String pathInfoResourcePath = this->getResourcePath(_path, pathInfo[0] == '/' ? &pathInfo[1] : pathInfo);
    if (_root[0] == '/') {
        return pathInfoResourcePath;
    }

    int cgiResourcePathSlashCount = countSlashes(cgiResourcePath);
    if (startsWith(_root, "./")) {
        cgiResourcePathSlashCount -= 1;
    }

    String res;
    for (int i = 0; i < cgiResourcePathSlashCount; i++) {
        res += "../";
    }
    res += pathInfoResourcePath;
    compressSlashes(res);
    return res;
}

void LocationBlock::setenvCgi(HttpRequest &req, const String &cgiResourcePath, const String &cgiScriptPath) {
    trySetenv("GATEWAY_INTERFACE", "CGI/1.1");
    trySetenv("SERVER_SOFTWARE", WEBSERV_V);
    String host = req.getHeader("Host");
    if (host != "") {
        trySetenv("SERVER_NAME", &(split(host, ":")[0])[0]);
    } else {
        tryUnsetenv("SERVER_NAME");
    }
    trySetenv("SERVER_PORT", req.getSocketPort());
    trySetenv("SERVER_PROTOCOL", req.getProtocol());
    trySetenv("REMOTE_ADDR", &req.getClientIpAddress()[0]);
    trySetenv("REMOTE_HOST", &req.getClientHostName()[0]);

    trySetenv("REQUEST_METHOD", req.getHttpMethod());
    trySetenv("QUERY_STRING", req.url().params);
    trySetenv("SCRIPT_NAME", cgiResourcePath);

    String pathInfo = parseCgiPathInfo(req, cgiScriptPath);
    trySetenv("PATH_INFO", pathInfo);
    if (pathInfo != "") {
        trySetenv("PATH_TRANSLATED", getPathTranslated(pathInfo, cgiResourcePath));
    } else {
        unsetenv("PATH_TRANSLATED");
    }

    if (req.hasBody()) {
        trySetenv("CONTENT_LENGTH", toString(req.getBodySize()));
        trySetenv("CONTENT_TYPE", req.getHeader("Content-Type"));
    } else {
        tryUnsetenv("CONTENT_LENGTH");
        tryUnsetenv("CONTENT_TYPE");
    }

    trySetenv("REDIRECT_STATUS", "200");  // Security needed to execute php-cgi

    trySetenv("REQUEST_URI", req.rawUrl());
    trySetenv("SCRIPT_FILENAME", parsePathFileName(cgiResourcePath));

    trySetenv("HTTP_COOKIE", req.getHeader("Cookie"));
}
