#include "Server.hpp"
#include "StaticClasses/MediaTypes.hpp"
#include "StaticClasses/StatusComments.hpp"
#include "blocks/Block.hpp"
#include "parsing/parsing.hpp"
#include "webserv.hpp"

std::map<int, String> StatusComments::_comments;
std::map<String, String> MediaTypes::_types;

// check autoindex
String generateDirectoryListingPage(const String &dir, String reqUrl, struct dirent *entry, DIR *dirStream) {
    if (lastChar(reqUrl) != '/') {
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
//// let client download files ✅    // for (int i = 0; i < 300; i++) {
//     ServerBlock *block = server->addBlock();
//     block->set("0.0.0.0", "5000", false);
//     for (int j = 0; j < 150; j++) {
//         LocationBlock *location = block->addLocation();
//         location->setPath("/");
//         location->setProxyPass("http://apple.com");
//     }
// }

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

// check delete folder
// check autoindex page
// check PollFd size

int main(int argc, char *argv[]) {
    Server *server = new Server();

    StatusComments::init();
    MediaTypes::init();
    signal(SIGINT, handleSigint);
    std::srand(std::time(0));
    isDebug = 0;
    g_conf_path = getRealtivePathToFile(argv[1]);

    if (parsing(argc, argv, server) == ERR) {
        delete server;
        return (1);
    }

    try {
        try {
            server->listen();
        } catch (char const *str) {
            std::cout << str << '\n';
        }
    } catch (const std::exception &e) {
        return 1;
    }

    delete server;
    return 0;
}
