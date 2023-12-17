#include "../parsing.hpp"
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

//// stream gateway responses
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
    throw "\nGracefully shutting down...";
}

int main(int argc, char *argv[]) {
    Server *server = new Server();

    StatusComments::init();
    MediaTypes::init();
    signal(SIGINT, handleSigint);
    std::srand(std::time(0));
    isDebug = 0;

    if (parsing(argc, argv, server) == ERR) {
        delete server;
        return (1);
    }

    g_conf_path = getRealtivePathToFile(argv[1]);

    // ServerBlock &block1 = server->addBlock();
    // block1.set("0.0.0.0", "3000", true);
    // // block1.setRoot("/Users/stan/42/vu/client/build");
    // block1.setRoot("www");
    // block1.setMaxBodySize(5000000);

    // LocationBlock &location1 = block1.addLocation();
    // location1.setPath("/", false);
    // location1.setIndex("index.html");
    // String test1[] = {"GET", "POST", "DELETE", ""};
    // String test2[] = {"GET", "POST", "DELETE", "PUT", ""};
    // location1.setAllowedMethods(test1);

    // LocationBlock &location2 = block1.addLocation();
    // location2.setPath("/api", false);
    // location2.setAllowedMethods(test2);
    // location2.setProxyPass("http://0.0.0.0:5000");

    try {
        server->listen();
    } catch (char const *str) {
        std::cout << str << '\n';
    }

    delete server;
    return 0;
}
