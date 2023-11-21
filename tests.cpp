#include <iostream>
// #include "webserv.hpp"

// void compressSlashesHandler(String str, String expected)
// {
//     String url = str;
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
#include <signal.h>
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

class Myclass {
   public:
    std::shared_ptr<int> yo;
    std::string data1;
    std::string data2;
    std::string data3;
    std::string data4;
    std::string data5;
    std::string data6;
    std::string data7;
    std::string data8;
    std::string data9;
    std::string data10;

    Myclass() : yo(new int(123)) {
    }

    void doSomething() {
        data1 = std::string(10000, 'a');
        data2 = std::string(10000, 'a');
        data3 = std::string(10000, 'a');
        data4 = std::string(10000, 'a');
        data5 = std::string(10000, 'a');
        data6 = std::string(10000, 'a');
        data7 = std::string(10000, 'a');
        data8 = std::string(10000, 'a');
        data9 = std::string(10000, 'a');
        data10 = std::string(10000, 'a');
    }

    void clear() {
        data1.clear();
        data2.clear();
        data3.clear();
        data4.clear();
        data5.clear();
        data6.clear();
        data7.clear();
        data8.clear();
        data9.clear();
        data10.clear();
    }

    void reset() {
        data1 = "";
        data2 = "";
        data3 = "";
        data4 = "";
        data5 = "";
        data6 = "";
        data7 = "";
        data8 = "";
        data9 = "";
        data10 = "";
    }
};

int main(int argc, char **argv, char **env) {
    // setenv("HEY", "", 1);
    if (unsetenv("shtkjwbfkjbn") != 0) {
        std::cout << "error\n";
    }
    // std::cout << getenv("HEY") << "\n";
    if (getenv("HEY") == NULL) {
        return 1;
    }
    return 0;
    // printf("%s\n", getenv("HEY"));
    // Myclass *hey = new Myclass();
    // std::shared_ptr<int> yo;

    // yo = hey->yo;
    // delete hey;
    // std::cout << *yo << '\n';
    // yo = NULL;
    // Myclass hey;

    // for (int i = 0; i < 100000; i++) {
    //     hey.doSomething();
    //     hey.reset();
    // }

    // Myclass hey;

    // for (int i = 0; i < 100000; i++) {
    //     hey.doSomething();
    //     hey.clear();
    // }

    // for (int i = 0; i < 10000; i++) {
    //     hey->doSomething();
    //     delete hey;
    //     hey = new Myclass();
    // }
    // delete hey;
    return 0;
}