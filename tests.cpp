#include <iostream>
// #include "webserv.hpp"

// void compressSlashesHandler(std::string str, std::string expected)
// {
//     std::string url = str;
//     compressSlashes(url);
//     if (url != expected)
//         std::cout << "Expected: " << expected << " Got: "<< url << "\n";
// }

// void compressSlashesTests()
// {
//     compressSlashesHandler("/api/truc", "/api/truc");
//     compressSlashesHandler("/api////truc", "/api/truc");
//     compressSlashesHandler("///////api////truc", "/api/truc");
//     compressSlashesHandler("///////////api////////truc", "/api/truc");
//     compressSlashesHandler("///////////api////////truc//", "/api/truc/");
//     compressSlashesHandler("///////////api////////truc/////////", "/api/truc/");
// }
#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

class Class {
   private:
    std::string str;

   public:
    Class() : str(100000, 'a') {
    }

    const std::string &f() {
        return str;
    }
};

int main(int argc, char **argv, char **env) {
    std::string str = "hey yo";

    // std::cout << str.substr(0, str.size() - 1) << '\n';
    std::cout << str.find("y", 2);
    // std::string str = "  ' ;; ;'  ='hey' ;attr='yo';    ; ; attr2='hey' ;   attr3;  'attr3'; 'sfb ';   ";
    // // std::string str = "'sfb'";
    // std::vector<std::string> res;
    // res.push_back("' ;; ;'  ='hey'");
    // res.push_back("attr='yo'");
    // res.push_back("");
    // res.push_back("");
    // res.push_back("attr2='hey'");
    // res.push_back("attr3");
    // res.push_back("'attr3'");

    // std::vector<std::string> items = split(str, ";");
    // for (int i = 0; i < items.size(); i++) {
    //     std::cout << "---" << items[i] << "---\n";
    //     std::cout << "---" << res[i] << "---\n";
    //     std::cout << "\n";
    // }
    // std::cout << std::strlen("Hey this is just a simple test file\nNothing fancy!\nSee ya") << "\n";

    // Class t;
    // for (int i = 0; i < 1000000; i++) {
    //     const std::string &str = t.f();
    // }
}