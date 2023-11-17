#include "Server.hpp"
#include "webserv.hpp"

bool startsWith(const std::string &str, const std::string &str2, size_t startPos) {
    return str.compare(startPos, str2.length(), str2) == 0;
}

void readNextEntry(DIR *dirStream, struct dirent **entry) {
    *entry = readdir(dirStream);
    if (!*entry && errno != 0) {
        debugErr("readdir", strerror(errno));
        closedir(dirStream);
        throw 500;
    }
}

std::string generateDirectoryListingPage(std::string dir, std::string reqUrl, struct dirent *entry, DIR *dirStream) {
    if (reqUrl.back() != '/') {
        reqUrl += "/";
    }

    std::string page = "<!DOCTYPE html><html><head>";
    page += "<title>Index of " + reqUrl + "</title></head><body ";
    // page += "style=\"background-color: #121212;color: white;font-family: monospace;\"";
    page += "><h1> Index of " + reqUrl + " </h1><hr><pre style='display: flex;flex-direction:column;'>";

    errno = 0;
    while (entry) {
        std::string entry_name = entry->d_name;
        if (entry_name != "." && checkPathAccess(dir) == 200) {
            if (isDirectory(dir + "/" + entry_name)) {
                entry_name += "/";
            }
            page += "<a href='" + reqUrl + entry_name + "'>" + entry_name + "</a>";
        }

        readNextEntry(dirStream, &entry);
    }

    page += "</pre><hr></body></html>";
    return page;
}

std::string getSocketPort(int socket) {
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);

    if (getsockname(socket, (struct sockaddr *)&sin, &len) == -1) {
        debugErr("getsockname", strerror(errno));
        throw 500;
    }
    return std::to_string(ntohs(sin.sin_port));
}

struct addrinfo *getOwnAddressInfo(const char *port) {
    struct addrinfo *res;
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;      // AF_INET or AF_INET6 to force version
    hints.ai_socktype = SOCK_STREAM;  // TCP or UDP (datagram)
    hints.ai_flags = AI_PASSIVE;      // assign the address of my local host to the socket structures

    int status = getaddrinfo(NULL, port, &hints, &res);
    if (status == -1) {
        debugErr("getaddrinfo", gai_strerror(errno));
        return NULL;
    }
    if (res == NULL) {
        debugErr("getaddrinfo", gai_strerror(errno));
    }
    return res;
}

int createBindedSocket(struct addrinfo *addrInfo) {
    int socketFd = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
    if (socketFd == -1) {
        debugErr("socket", strerror(errno));
        return -1;
    }
    // fcntl(socketFd, F_SETFL, O_NONBLOCK);
    return socketFd;
}

int bindSocket(int socketFd, struct addrinfo *addrInfo) {
    int yes = 1;
    if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        debugErr("setsockopt", strerror(errno));
        return -1;
    }
    int status = ::bind(socketFd, addrInfo->ai_addr, addrInfo->ai_addrlen);
    if (status == -1) {
        debugErr("bind", strerror(errno));
        return -1;
    }
    return 0;
}

int listenToSocket(int socketFd, std::string port) {
    int status = listen(socketFd, 20);
    if (status == -1) {
        debugErr("listen", strerror(errno));
        return -1;
    }
    std::cout << "Server is listening on port " << port << "...\n";
    return 0;
}

int getFileContent(std::string path, std::string &buf) {
    std::ifstream file(path);
    if (!file) {
        return 1;
    }

    std::stringstream fileContent;
    fileContent << file.rdbuf();
    buf = fileContent.str();
    file.close();
    return 0;
}

std::string getFileExtension(std::string fileName) {
    int lastSlashPos = fileName.find_last_of('/');
    int dotPos = fileName.find_last_of(".");

    if (dotPos == -1 || dotPos < lastSlashPos) {
        return "undefined";
    }
    return fileName.substr(dotPos);
}

int checkPathAccess(std::string path) {
    if (access(&path[0], F_OK) == -1) {
        return 404;
    }
    if (access(&path[0], R_OK) == -1) {
        return 403;
    }
    return 200;
}

void getMethod(LocationBlock &location, HttpRequest &req, HttpResponse &res) {
    std::string resourcePath = location.getResourcePath(req);

    int accessStatus = checkPathAccess(resourcePath);
    if (accessStatus != 200) {
        throw accessStatus;
    }

    if (!isDirectory(resourcePath)) {
        res.loadFile(200, resourcePath);
        return;
    }

    try {
        res.loadFile(200, location.getResourcePath(req, location.getIndex()));
    } catch (int status) {
        if (status == 500) {
            throw 500;
        }
        if (location.isAutoIndex() == false) {
            throw 403;
        }
        res.listDirectory(resourcePath, req.getUrl());
    }
}

std::string getFormBoundary(HttpRequest &req) {
    std::vector<std::string> values = split(req.getHeader("Content-Type"), ";");
    if (values.size() < 2 || values[0] != "multipart/form-data") {
        debug("400: 4", "", YELLOW);
        throw 400;
    }

    std::vector<std::string> boundary = split(values[1], "=");
    if (boundary.size() != 2 || boundary[0] != "boundary" || boundary[1] == "") {
        debug("400: 5", "", YELLOW);
        throw 400;
    }
    return boundary[1];
}

std::string getFormFileName(HttpRequest &req, size_t from, size_t to) {
    std::vector<std::string> headerValues = split(req.findBodyHeader("Content-Disposition", from, to), ";");
    if (headerValues.size() < 3 || headerValues[0] != "form-data") {
        debug("400: 6", "", YELLOW);
        throw 400;
    }

    std::vector<std::string> filenameKey = split(headerValues[2], "=");
    if (filenameKey.size() != 2 || filenameKey[0] != "filename" || filenameKey[1] == "\"\"") {
        debug("400: 7", "", YELLOW);
        throw 400;
    }

    std::string filename = filenameKey[1].substr(1, filenameKey[1].size() - 2);
    percentDecode(filename);
    return filename;
}

void createFile(const std::string &name, const char *data, size_t size) {
    std::ofstream file(&name[0], std::ios::binary);
    if (!file) {
        throw 500;
    }

    debug("filename", name, CYAN);
    debug("file size", std::to_string(size), CYAN);
    file.write(data, size);
    file.close();
}

bool findFormBoundaries(HttpRequest &req, const std::string &boundary, size_t &currFormPos, size_t &formHeadersEndPos, size_t &formBodyEndPos) {
    currFormPos = req.searchBody(boundary, currFormPos);
    if (currFormPos == (size_t)-1) {
        debug("400: 1", "", YELLOW);
        throw 400;
    }
    currFormPos += boundary.size();
    if (startsWith(req.getBody(), "--", currFormPos)) {
        return true;
    }

    formBodyEndPos = req.searchBody(boundary, currFormPos);
    if (formBodyEndPos == (size_t)-1) {
        debug("400: 3", "", YELLOW);
        throw 400;
    }

    formHeadersEndPos = req.searchBody(CRLF_CRLF, currFormPos + 1, formBodyEndPos);
    if (formHeadersEndPos == (size_t)-1) {
        debug("400: 2", "", YELLOW);
        throw 400;
    }
    formHeadersEndPos += 4;
    return false;
}

void postMethod(LocationBlock &location, HttpRequest &req, HttpResponse &res) {
    std::string boundary = "--" + getFormBoundary(req);
    size_t currFormPos = 0;
    size_t formHeadersEndPos = 0;
    size_t formBodyEndPos = 0;
    std::string filePath;

    while (true) {
        debug("BEGIN FORM", "", YELLOW);
        bool isEnd = findFormBoundaries(req, boundary, currFormPos, formHeadersEndPos, formBodyEndPos);
        if (isEnd) {
            debug("END OF FORM", "", YELLOW);
            break;
        }

        std::string filename = getFormFileName(req, currFormPos, formHeadersEndPos);
        filePath = location.getUploadFilePath(filename);
        if (checkPathAccess(filePath) != 404) {
            throw 409;
        }
        // if (filename.find('/') != (size_t)-1) {
        //     throw 403;
        // }

        size_t fileSize = (formBodyEndPos - formHeadersEndPos) - 2;
        createFile(filePath, &req.getBody()[formHeadersEndPos], fileSize);
    }

    res.loadFile(200, filePath);
}

void deleteMethod(LocationBlock &location, HttpRequest &req, HttpResponse &res) {
    std::string resourcePath = location.getResourcePath(req);
    debug("deleting", resourcePath, YELLOW);

    if (checkPathAccess(resourcePath) == 404) {
        throw 404;
    }
    if (isDirectory(resourcePath)) {
        throw 403;
    }
    if (std::remove(&resourcePath[0]) != 0) {
        throw 500;
    }

    res.loadFile(200, "defaultPages/delete_success.html");
}

// TO DO:
//// let client download files ✅
//// upload files on server ✅
//// cookies ✅
//// continuous parsing ✅
//// content-encoding: chunked ✅
//// CGI envs
//// url params ?&2
//// Parsing utils
//// 415 Unsupported Media Type
//// Range?
//// get SIGINT to exit nice and clean
//// Expect: 100-continue
//// proxy_pass
//// refacto

void exitProgram(Server &server) {
    delete &server;
    debug("-------", "", RED);
    debug("EXITING", "", RED);
    debug("-------", "", RED);
    exit(0);
}

extern std::string g_conf_path;
extern char **g_env;

std::string getRealtivePathToFile(std::string path) {
    int lastSlash = path.find_last_of("/");
    if (lastSlash == -1)
        return "";

    return path.substr(0, lastSlash + 1);
}

int main(int argc, char *argv[], char **env) {
    Server *server = new Server();

    if (argc != 2) {
        debugErr("Configuration file required", "");
        return 1;
    }

    g_env = env;
    std::srand(std::time(0));
    g_conf_path = getRealtivePathToFile(argv[1]);
    server->addBlocks(1);

    server->getServerBlock(0).set("3000", true);
    server->getServerBlock(0).setHostName("localhost");
    server->getServerBlock(0).setIndex("index.html");
    server->getServerBlock(0).setRoot("www");
    server->getServerBlock(0).addErrorPage(404, "/404.html");
    server->getServerBlock(0).setSessionCookie("sessionId");

    // server->getServerBlock(1).set("3000", false);
    // server->getServerBlock(1).setHostName("virtual.org");
    // server->getServerBlock(1).setIndex("index.html");
    // server->getServerBlock(1).setRoot("www");

    server->addLocationBlocks(0, 5);

    server->getLocationBlock(0, 0).setPath("/");
    server->getLocationBlock(0, 0).setAutoIndex(true);
    server->getLocationBlock(0, 0).setHandlers(getMethod, postMethod, NULL);
    server->getLocationBlock(0, 0).addCgiExtension(".sh", "/bin/sh");
    server->getLocationBlock(0, 0).addCgiExtension(".py", "/usr/bin/python3");
    // server->getLocationBlock(0, 0).setHeader("Set-Cookie", "cookie=123; Path=/folder");

    server->getLocationBlock(0, 1).setPath("////folder////");
    server->getLocationBlock(0, 1).setHandlers(getMethod, postMethod, NULL);
    server->getLocationBlock(0, 1).setAutoIndex(true);
    server->getLocationBlock(0, 1).addHeader("Set-Cookie", "cookie=123; Path=/folder");
    server->getLocationBlock(0, 1).addErrorPage(404, "404.html");
    // server->getLocationBlock(0, 1).setRedirection(303, "/");

    server->getLocationBlock(0, 2).setPath("/upload", false);
    server->getLocationBlock(0, 2).setHandlers(getMethod, postMethod, deleteMethod);
    server->getLocationBlock(0, 2).setUploadRoot("www/upload");
    server->getLocationBlock(0, 2).setAutoIndex(true);
    server->getLocationBlock(0, 2).addCgiExtension(".py", "/usr/bin/python3");
    server->getLocationBlock(0, 2).setMaxBodySize(2000000000);

    server->getLocationBlock(0, 3).setPath("/download", false);
    server->getLocationBlock(0, 3).addHeader("Content-Disposition", "attachement");
    server->getLocationBlock(0, 3).setHandlers(getMethod, NULL, NULL);

    server->getLocationBlock(0, 4).setPath("/folder/dir", false);
    server->getLocationBlock(0, 4).setHandlers(getMethod, NULL, NULL);

    if (server->listen() == -1) {
        return 1;
    }
    delete &server;
    return 0;
}

void debugErr(const std::string &title, const char *err) {
    std::cerr << RED << title << ": " << err << "\n"
              << WHITE;
}

void debug(std::string title, std::string arg, std::string color) {
    // return;
    std::cout << color << title;
    if (arg != "") {
        std::cout << ": " << arg;
    }
    std::cout << "\n"
              << WHITE;
}