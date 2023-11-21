#include "MediaTypes.hpp"
#include "Server.hpp"
#include "StatusComments.hpp"
#include "blocks/Block.hpp"
#include "webserv.hpp"

bool StatusComments::_isInit = false;
std::unordered_map<int, String> StatusComments::_comments;

bool MediaTypes::_isInit = false;
std::unordered_map<String, String> MediaTypes::_types;

String generateDirectoryListingPage(const String &dir, String reqUrl, struct dirent *entry, DIR *dirStream) {
    if (reqUrl.back() != '/') {
        reqUrl += "/";
    }

    String page = "<!DOCTYPE html><html><head>";
    page += "<title>Index of " + reqUrl + "</title></head><body ";
    // page += "style=\"background-color: #121212;color: white;font-family: monospace;\"";
    page += "><h1> Index of " + reqUrl + " </h1><hr><pre style='display: flex;flex-direction:column;'>";

    errno = 0;
    while (entry) {
        String entry_name = entry->d_name;
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

void getMethod(LocationBlock &location, HttpRequest &req, HttpResponse &res) {
    String resourcePath = location.getResourcePath(req.url().path);

    int accessStatus = checkPathAccess(resourcePath);
    if (accessStatus != 200) {
        throw accessStatus;
    }

    if (!isDirectory(resourcePath)) {
        res.loadFile(200, resourcePath);
        return;
    }

    try {
        res.loadFile(200, location.getResourcePath(req.url().path, location.getIndex()));
    } catch (int status) {
        if (status == 500) {
            throw 500;
        }
        if (location.isAutoIndex() == false) {
            throw 403;
        }
        res.listDirectory(resourcePath, req.url().path);
    }
}

String getFormBoundary(HttpRequest &req) {
    std::vector<String> values = split(req.getHeader("Content-Type"), ";");
    if (values.size() < 2 || values[0] != "multipart/form-data") {
        debug("400: 4", "", YELLOW);
        throw 400;
    }

    std::vector<String> boundary = split(values[1], "=");
    if (boundary.size() != 2 || boundary[0] != "boundary" || boundary[1] == "") {
        debug("400: 5", "", YELLOW);
        throw 400;
    }
    return boundary[1];
}

String getFormFileName(HttpRequest &req, size_t from, size_t to) {
    std::vector<String> headerValues = split(req.findBodyHeader("Content-Disposition", from, to), ";");
    if (headerValues.size() < 3 || headerValues[0] != "form-data") {
        debug("400: 6", "", YELLOW);
        throw 400;
    }

    std::vector<String> filenameKey = split(headerValues[2], "=");
    if (filenameKey.size() != 2 || filenameKey[0] != "filename" || filenameKey[1] == "\"\"") {
        debug("400: 7", "", YELLOW);
        throw 400;
    }

    String filename = filenameKey[1].substr(1, filenameKey[1].size() - 2);
    percentDecode(filename);
    return filename;
}

void createFile(const String &name, const char *data, size_t size) {
    std::ofstream file(&name[0], std::ios::binary);
    if (!file) {
        throw 500;
    }

    debug("filename", name, CYAN);
    debug("file size", std::to_string(size), CYAN);
    file.write(data, size);
    file.close();
}

bool findFormBoundaries(HttpRequest &req, const String &boundary, size_t &currFormPos, size_t &formHeadersEndPos, size_t &formBodyEndPos) {
    currFormPos = req.searchBody(boundary, currFormPos);
    if (currFormPos == NPOS) {
        debug("400: 1", "", YELLOW);
        throw 400;
    }
    currFormPos += boundary.size();
    if (startsWith(req.getBody(), "--", currFormPos)) {
        return true;
    }

    formBodyEndPos = req.searchBody(boundary, currFormPos);
    if (formBodyEndPos == NPOS) {
        debug("400: 3", "", YELLOW);
        throw 400;
    }

    formHeadersEndPos = req.searchBody(CRLF_CRLF, currFormPos + 1, formBodyEndPos);
    if (formHeadersEndPos == NPOS) {
        debug("400: 2", "", YELLOW);
        throw 400;
    }
    formHeadersEndPos += 4;
    return false;
}

void postMethod(LocationBlock &location, HttpRequest &req, HttpResponse &res) {
    String boundary = "--" + getFormBoundary(req);
    size_t currFormPos = 0;
    size_t formHeadersEndPos = 0;
    size_t formBodyEndPos = 0;
    String filePath;

    while (true) {
        debug("BEGIN FORM", "", YELLOW);
        bool isEnd = findFormBoundaries(req, boundary, currFormPos, formHeadersEndPos, formBodyEndPos);
        if (isEnd) {
            debug("END OF FORM", "", YELLOW);
            break;
        }

        String filename = getFormFileName(req, currFormPos, formHeadersEndPos);
        filePath = location.getUploadFilePath(filename);
        if (checkPathAccess(filePath) != 404) {
            throw 409;
        }
        // if (filename.find('/') != NPOS) {
        //     throw 403;
        // }

        size_t fileSize = (formBodyEndPos - formHeadersEndPos) - 2;
        createFile(filePath, &req.getBody()[formHeadersEndPos], fileSize);
    }

    res.loadFile(200, filePath);
}

void deleteMethod(LocationBlock &location, HttpRequest &req, HttpResponse &res) {
    String resourcePath = location.getResourcePath(req.url().path);
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
//// url params ?& ✅
//// PATH_INFO ✅
//// kill() cgi process if client gets kicked out? ✅
//// allow user to choose ip address for server ✅
//// persistant connexions ✅

//// get SIGINT to exit nice and clean
//// Parsing utils
//// 415 Unsupported Media Type
//// Range?
//// Expect: 100-continue
//// proxy_pass
//// refacto

String getRealtivePathToFile(String path) {
    int lastSlash = path.find_last_of("/");
    if (lastSlash == -1) {
        return "";
    }

    return path.substr(0, lastSlash + 1);
}

int main(int argc, char *argv[]) {
    Server *server = new Server();

    if (argc != 2) {
        debugErr("Configuration file required", "");
        delete server;
        return 1;
    }

    std::srand(std::time(0));
    g_conf_path = getRealtivePathToFile(argv[1]);
    server->addBlocks(1);

    // server->getServerBlock(0).set("0.0.0.0", "3000", true);
    // server->getServerBlock(0).setIndex("index.html");
    // server->getServerBlock(0).setRoot("www");

    // server->addLocationBlocks(0, 1);

    // server->getLocationBlock(0, 0).setPath("/", false);
    // server->getLocationBlock(0, 0).setHandlers(getMethod, NULL, NULL);
    // server->getLocationBlock(0, 0).addCgiCommand(".cgi", "");
    // server->getLocationBlock(0, 0).addCgiCommand(".py", "/usr/bin/python3");

    ///////////////////////////////////////////////////////////////
    server->getServerBlock(0).set("192.168.0.176", "3000", true);
    server->getServerBlock(0).set("0.0.0.0", "3000", true);
    server->getServerBlock(0).setHostName("localhost");
    server->getServerBlock(0).setIndex("index.html");
    server->getServerBlock(0).setRoot("www");
    server->getServerBlock(0).addErrorPage(404, "/404.html");
    server->getServerBlock(0).setSessionCookie("sessionId");

    // server->getServerBlock(1).set("3000", false);
    // server->getServerBlock(1).setHostName("virtual.org");
    // server->getServerBlock(1).setIndex("index.html");
    // server->getServerBlock(1).setRoot("www");

    server->addLocationBlocks(0, 4);

    server->getLocationBlock(0, 0).setPath("/", false);
    server->getLocationBlock(0, 0).setAutoIndex(true);
    server->getLocationBlock(0, 0).setHandlers(getMethod, postMethod, NULL);
    server->getLocationBlock(0, 0).addCgiCommand(".sh", "/bin/sh");
    server->getLocationBlock(0, 0).addCgiCommand(".py", "/usr/bin/python3");
    // server->getLocationBlock(0, 0).setHeader("Set-Cookie", "cookie=123; Path=/folder");

    server->getLocationBlock(0, 1).setPath("////folder////");
    server->getLocationBlock(0, 1).setHandlers(getMethod, postMethod, NULL);
    server->getLocationBlock(0, 1).setAutoIndex(true);
    // server->getLocationBlock(0, 1).addHeader("Set-Cookie", "cookie=123; Path=/folder");
    server->getLocationBlock(0, 1).addErrorPage(404, "404.html");
    server->getLocationBlock(0, 1).setRedirection(303, "/");

    server->getLocationBlock(0, 2).setPath("/upload", false);
    server->getLocationBlock(0, 2).setHandlers(getMethod, postMethod, deleteMethod);
    server->getLocationBlock(0, 2).setUploadRoot("www/upload");
    server->getLocationBlock(0, 2).setAutoIndex(true);
    server->getLocationBlock(0, 2).addCgiCommand(".py", "/usr/bin/python3");
    server->getLocationBlock(0, 2).setMaxBodySize(2000000000);

    server->getLocationBlock(0, 3).setPath("/download", false);
    server->getLocationBlock(0, 3).addHeader("Content-Disposition", "attachement");
    server->getLocationBlock(0, 3).setHandlers(getMethod, NULL, NULL);

    if (server->listen() == -1) {
        return 1;
    }
    delete server;
    return 0;
}
