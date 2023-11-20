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
#define _XOPEN_SOURCE_EXTENDED 1

int main(int argc, char **argv, char **env) {
    int pipes[2];
    int pipesRead[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, pipes);
    socketpair(AF_UNIX, SOCK_STREAM, 0, pipesRead);
    // fcntl(pipes[0], F_SETFL, O_NONBLOCK);
    // int bufsize = 2500 * 2500;
    // setsockopt(pipes[0], SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof(bufsize));
    // setsockopt(pipes[1], SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof(bufsize));
    // socketpair(AF_INET, SOCK_STREAM, 0, pipesRead);

    int pid = fork();
    if (pid == 0) {
        dup2(pipes[0], 0);
        dup2(pipesRead[1], 1);

        close(pipes[0]);
        close(pipes[1]);
        close(pipesRead[0]);
        close(pipesRead[1]);

        char *const args[] = {(char *)"/usr/bin/python3", (char *)"./www/cgi/hello.py", NULL};
        if (execve(*args, args, env) == -1) {
            std::cerr << strerror(errno) << "\n";
        }

        // char buf[100 + 1];
        // std::cerr << "reading: ";
        // // std::cerr << buf << '\n';

        // buf[read(0, buf, 100)] = '\0';
        // std::cerr << buf << '\n';

        // // std::string buf(5, '\0');
        // // read(0, &buf[0], 5);
        // std::cerr << "sending from child..." << '\n';
        // write(1, "hey!", 4);
        // std::cerr << "done writting." << '\n';
        return 0;
    }

    sleep(2);

    std::string data(5, 'a');
    std::cerr << "sending..." << '\n';
    if (write(pipes[1], &data[0], 5) == -1) {
        std::cerr << "write failed\n";
    }
    close(pipes[1]);
    close(pipes[0]);

    waitpid(pid, NULL, 0);
    std::cout << "process has finished.\n";

    struct pollfd el = {pipesRead[0], POLLIN | POLLOUT, 0};
    poll(&el, 1, -1);
    std::cout << "Writable" << (el.revents & POLLOUT) << "\n";
    std::cout << "Readable" << (el.revents & POLLIN) << "\n";

    char buf[100 + 1];
    int readBytes = read(pipesRead[0], buf, 100);
    buf[readBytes] = '\0';
    std::cout << readBytes << '\n';
    std::cerr << buf << "\n";

    poll(&el, 1, -1);
    std::cout << "Writable" << (el.revents & POLLOUT) << "\n";
    std::cout << "Readable" << (el.revents & POLLIN) << "\n";
    // } else {
    //     std::cout << "not readable\n";
    // }

    // close(pipesRead[1]);
    // close(pipesRead[0]);
    return 0;

    // std::cout << str.substr(0, str.size() - 1) << '\n';
    // std::cout << (str.find("y", -1) == (size_t)-1) << "\n";
    // std::cout << str.rfind() << "\n";
    // String str = "  ' ;; ;'  ='hey' ;attr='yo';    ; ; attr2='hey' ;   attr3;  'attr3'; 'sfb ';   ";
    // // String str = "'sfb'";
    // std::vector<String> res;
    // res.push_back("' ;; ;'  ='hey'");
    // res.push_back("attr='yo'");
    // res.push_back("");
    // res.push_back("");
    // res.push_back("attr2='hey'");
    // res.push_back("attr3");
    // res.push_back("'attr3'");

    // std::vector<String> items = split(str, ";");
    // for (int i = 0; i < items.size(); i++) {
    //     std::cout << "---" << items[i] << "---\n";
    //     std::cout << "---" << res[i] << "---\n";
    //     std::cout << "\n";
    // }
    // std::cout << std::strlen("Hey this is just a simple test file\nNothing fancy!\nSee ya") << "\n";

    // Class t;
    // for (int i = 0; i < 1000000; i++) {
    //     const String &str = t.f();
    // }
}