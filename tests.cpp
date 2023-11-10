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

#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <dirent.h>
#include <sys/types.h>
#include <filesystem>

int main()
{
    std::string str;

    str.resize(10, '\0');
    // str[5] = '\0';

    std::cout << str.length() << '\n';
    std::cout << std::strlen(&str[0]) << '\n';
    // int fds[2];
    // pipe(fds);

    // write(fds[1], "hello", 6);
    // // close(fds[1]);

    // struct pollfd fd;
    // fd.fd = fds[0];
    // fd.events = POLLIN | POLLOUT;

    // while (true)
    // {
    //     poll(&fd, 1, -1);
    //     if (fd.revents & POLLIN)
    //     {
    //         std::cout << "can read from it\n";
    //         char buf[100];
    //         int readBytes = read(fd.fd, buf, 100);
    //         std::cout << readBytes << '\n';
    //     }
    //     if (fd.revents == POLLOUT)
    //     {
    //         std::cout << "can write to it\n";
    //     }
    // }
}