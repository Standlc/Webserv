#include "Server.hpp"
#include "StaticClasses/MediaTypes.hpp"
#include "StaticClasses/StatusComments.hpp"
#include "blocks/Block.hpp"
#include "parsing/parsing.hpp"
#include "webserv.hpp"

std::map<int, String> StatusComments::_comments;
std::map<String, String> MediaTypes::_types;

int main(int argc, char *argv[]) {
    Server server;

    StatusComments::init();
    MediaTypes::init();
    signal(SIGINT, handleSigint);
    std::srand(std::time(0));
    isDebug = 0;
    g_conf_path = getRealtivePathToFile(argc == 2 ? argv[1] : "");

    if (parsing(argc, argv, &server) == ERR) {
        return (1);
    }

    try {
        try {
            server.listen();
        } catch (char const *str) {
            std::cout << str << '\n';
        }
    } catch (const std::exception &e) {
        return 1;
    }

    return 0;
}
