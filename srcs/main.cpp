#include "Server.hpp"
#include "StaticClasses/MediaTypes.hpp"
#include "StaticClasses/StatusComments.hpp"
#include "blocks/Block.hpp"
#include "webserv.hpp"

std::unordered_map<int, String> StatusComments::_comments;
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

//// get SIGINT to exit nice and clean ✅
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
    // throw SigintError();
    throw "\nSee ya!";
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        debugErr("A configuration file is required as first argument");
        return 1;
    }

    Server *server = new Server();
    StatusComments::init();
    MediaTypes::init();

    signal(SIGINT, handleSigint);
    std::srand(std::time(0));
    g_conf_path = getRealtivePathToFile(argv[1]);
    server->addBlocks(1);

    server->getServerBlock(0).set("0.0.0.0", "80", true);
    server->getServerBlock(0).setRoot("/Users/stan/Desktop/fsh/client/build");
    server->getServerBlock(0).addLocationBlocks(2);

    server->getLocationBlock(0, 0).setPath("/", false);
    server->getLocationBlock(0, 0).setIndex("index.html");
    server->getLocationBlock(0, 0).setAllowedMethods((String[]){"GET", "POST", "DELETE", ""});

    server->getLocationBlock(0, 1).setPath("/api", false);
    server->getLocationBlock(0, 1).setAllowedMethods((String[]){"GET", "POST", "DELETE", "PUT"});
    server->getLocationBlock(0, 1).setProxyPass("http://0.0.0.0:5000");

    try {
        server->listen();
    } catch (char const *str) {
        std::cout << str << '\n';
    }

    delete server;
    return 0;
}
