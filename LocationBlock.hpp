#ifndef LOCATION_BLOCK_HPP
#define LOCATION_BLOCK_HPP

#include "Block.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "PollFd.hpp"
#include "Server.hpp"
#include "webserv.hpp"

class Server;
class ServerBlock;
class ClientPollFd;
class LocationBlock;

typedef void (*pathHandlerType)(LocationBlock &block, HttpRequest &req, HttpResponse &res);

bool isUnkownMethod(std::string method) {
    return method != "GET" && method != "POST" && method != "DELETE";
}

int pipeAndFork(int pipes[2]) {
    if (pipe(pipes)) {
        debugErr("pipe", strerror(errno));
        return -1;
    }

    int pid = fork();
    if (pid == -1) {
        debugErr("fork", strerror(errno));
        close(pipes[0]);
        close(pipes[1]);
        return -1;
    }
    return pid;
}

int isDirectory(std::string path) {
    struct stat pathInfo;

    if (stat(&path[0], &pathInfo) == -1) {
        debugErr("stat", strerror(errno));
        throw 500;
    }

    return (pathInfo.st_mode & S_IFDIR) != 0;
}

class LocationBlock : public Block {
   private:
    std::string _path;
    bool _isExact;
    std::map<std::string, pathHandlerType> _handlers;

   public:
    LocationBlock(const Block &b) : Block(b), _isExact(false){};

    LocationBlock &operator=(const LocationBlock &b) {
        Block::operator=(b);
        _path = b._path;
        _isExact = b._isExact;
        _handlers = b._handlers;
        return *this;
    }

    clientPollFdHandlerType execute(Server &server, ClientPollFd &client);
    clientPollFdHandlerType handleCgi(Server &server, ClientPollFd &client);

    int execveCgi(Server &server, ClientPollFd &client, int cgiReqPipes[2], std::string cgiFile) {
        int cgiResPipes[2];
        int pid = pipeAndFork(cgiResPipes);
        if (pid == -1) {
            return -1;
        }

        if (pid == 0) {
            this->handleCgiChildProcess(cgiReqPipes, cgiResPipes, cgiFile);
            exitProgram(server);
        }
        client.setUpCgi(pid, cgiResPipes);
        return 0;
    }

    void handleCgiChildProcess(int reqPipes[2], int resPipes[2], std::string filename) {
        if (reqPipes != NULL) {
            if (dup2(reqPipes[0], 0) == -1) {
                debugErr("dup2", strerror(errno));
            }
            close(reqPipes[1]);
        }

        if (dup2(resPipes[1], 1) == -1) {
            debugErr("dup2", strerror(errno));
        }
        close(resPipes[0]);

        if (chdir(&(_root[0])) == -1) {
            debugErr("chdir", strerror(errno));
        }
        std::string command = _cgiExtensions[getFileExtension(filename)];
        char *const args[] = {(char *)&command[0], (char *)&filename[0], NULL};

        if (execve(args[0], args, g_env) == -1) {
            debugErr("execve", strerror(errno));
        }
    }

    // void proxyPass(Server &server, ClientPollFd &client) {
    //     // Change host name
    //     // change connection to close
    //     // Send 502 if request fails or res status is error
    // }

    void throwReqErrors(HttpRequest &req) {
        std::string reqHttpMethod = req.getHttpMethod();
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
        // if (req.hasBody() && reqHttpMethod != "POST") {
        //     throw 400;
        // }
    }

    std::string assembleRedirectionUrl(HttpRequest &req) {
        if (_redirection.url.compare(0, 5, "http:") == 0 || _redirection.url.compare(0, 6, "https:") == 0) {
            return _redirection.url;
        }
        if (_redirection.url[0] == '/') {
            return "http://" + req.getHeader("Host") + _redirection.url;
        }
        return _redirection.url;
    }

    bool handlesCgiExtension(std::string extension) {
        return _cgiExtensions.find(extension) != _cgiExtensions.end();
    }

    bool exceedsReqMaxSize(size_t size) {
        if (_reqBodyMaxSize == (size_t)-1) {
            return false;
        }
        return size > _reqBodyMaxSize;
    }

    std::string generateSessionCookie() {
        std::string chars = "0123456789abcdef";
        std::string cookie;
        for (int i = 0; i < 6; i++) {
            cookie += chars[(int)(std::rand() / (float)RAND_MAX * 15)];
        }
        return cookie;
    }

    void setPath(const std::string &path, bool isExact = false) {
        if (path[0] != '/') {
            _path = "/" + path;
        }
        _path = path;
        compressSlashes(_path);
        _isExact = isExact;
    }

    void setHandlers(pathHandlerType getMethod, pathHandlerType postMethod, pathHandlerType deleteMethod) {
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

    bool handlesHttpMethod(std::string httpMethod) {
        return _handlers.find(httpMethod) != _handlers.end();
    }

    const std::string &getPath() {
        return _path;
    }

    bool isExactPath() {
        return _isExact;
    }

    const std::string &getIndex() {
        return _index;
    }

    bool isAutoIndex() {
        return _autoIndex;
    }
};

#endif