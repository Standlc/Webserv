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
//// check socket ip to pick the right server config ✅
//// proxy_pass ✅
//// cgi status header ✅
//// cgi local redirects ✅
//// protect Content-Length overflows ✅
//// decode transfer encoded cgi output ✅

//// get SIGINT to exit nice and clean
//// make less data copies
//// check cookies
//// Range?
//// Expect: 100-continue
//// refacto

String getRealtivePathToFile(String path) {
    int lastSlash = path.find_last_of("/");
    if (lastSlash == -1) {
        return "";
    }
    return path.substr(0, lastSlash + 1);
}

void handleSigint(int sig) {
    (void)sig;
    throw SigintError();
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        debugErr("A configuration file is required as first argument");
        return 1;
    }

    Server *server = new Server();

    // signal(SIGINT, handleSigint);
    std::srand(std::time(0));
    g_conf_path = getRealtivePathToFile(argv[1]);
    server->addBlocks(2);

    ///////////////////////////////////////////////////////////////

    server->getServerBlock(0).set("", "3000", true);
    server->getServerBlock(0).setIndex("index.html");
    server->getServerBlock(0).setRoot("www");
    server->getServerBlock(0).addErrorPage(404, "/404.html");
    server->getServerBlock(0).setSessionCookie("sessionId");
    server->getServerBlock(0).addLocationBlocks(4);

    server->getLocationBlock(0, 0).setPath("/", false);
    server->getLocationBlock(0, 0).setAutoIndex(true);
    server->getLocationBlock(0, 0).setAllowedMethods((String[]){"GET", "POST", "delee", ""});
    server->getLocationBlock(0, 0).addCgiCommand(".sh", "/bin/sh");
    server->getLocationBlock(0, 0).addCgiCommand(".py", "/usr/bin/python3");

    server->getLocationBlock(0, 1).setPath("////folder////");
    server->getLocationBlock(0, 1).setAllowedMethods((String[]){"GET", "POST", "", ""});
    server->getLocationBlock(0, 1).setAutoIndex(true);
    server->getLocationBlock(0, 1).addErrorPage(404, "404.html");
    // server->getLocationBlock(0, 1).setRedirection(303, "/");

    server->getLocationBlock(0, 2).setPath("/upload", false);
    server->getLocationBlock(0, 2).setAllowedMethods((String[]){"GET", "POST", "DELETE", ""});
    server->getLocationBlock(0, 2).setUploadRoot("www/upload");
    server->getLocationBlock(0, 2).setAutoIndex(true);
    server->getLocationBlock(0, 2).addCgiCommand(".py", "/usr/bin/python3");
    server->getLocationBlock(0, 2).setMaxBodySize(2000000000);

    server->getLocationBlock(0, 3).setPath("/download", false);
    server->getLocationBlock(0, 3).addHeader("Content-Disposition", "attachement");
    server->getLocationBlock(0, 3).setAllowedMethods((String[]){"GET", "", "", ""});

    //

    server->getServerBlock(1).set("192.168.0.176", "3000", true);
    server->getServerBlock(1).setRoot("www/folder");
    server->getServerBlock(1).addLocationBlocks(1);
    server->getServerBlock(1).addHostName("virtual.org");

    server->getLocationBlock(1, 0).setPath("/", false);
    server->getLocationBlock(1, 0).setAllowedMethods((String[]){"GET", "POST", "DELETE", ""});
    server->getLocationBlock(1, 0).setProxyPass("http://localhost:8080");
    // server->getLocationBlock(1, 0).setProxyPass("http://example.com");

    try {
        server->listen();
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
    }

    delete server;
    return 0;
}
