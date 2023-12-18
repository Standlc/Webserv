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
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <map>
#include <vector>

class Myclass {
   public:
    void f1(int n) {
        std::cout << "f1()\n";
    }
    void f2(int n) {
        std::cout << "f2()\n";
    }
    // std::function<void(MyClass *, int)> func
    // template <typename ClassType>
    void function(int n, void (Myclass::*f)(int)) {
        (this->*f)(n);
    }

    void handler(int n) {
        if (n > 0) {
            function(n, (&Myclass::f1));
        }
    }
};

class GradeTooLowException : public std::exception {
    const char *what() const throw() {
        return "The grade is too low";
    }
};
void handler(int sig) {
    throw GradeTooLowException();
    std::cout << sig << "\n";
    exit(0);
}

// void something() {
//     std::cout << "first something\n";
//     std::cout << "something\n";
//     std::cout << "something\n";
//     std::cout << "something\n";
//     std::cout << "something\n";
//     std::cout << "something\n";
//     std::cout << "something\n";
//     std::cout << "last something\n";
//     std::cout << std::endl;
// }
struct addrinfo *getServerAddressInfo(std::string serverIpAddress, std::string port) {
    struct addrinfo *res;
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int status = getaddrinfo(&serverIpAddress[0], &port[0], &hints, &res);
    if (status != 0) {
        std::cerr << gai_strerror(errno) << '\n';
        throw 1;
    }
    return res;
}

void checkPoll(int fd) {
    struct pollfd readable = {fd, POLLIN | POLLOUT, 0};
    poll(&readable, 1, -1);
    if ((readable.revents & POLLIN) == 1) {
        std::cerr << "readable" << std::endl;
    }
    if ((readable.revents & POLLOUT) == 4) {
        std::cerr << "writable" << std::endl;
    }
}

typedef std::multimap<int, int> stuff;

void f(std::multimap<int, int> *m) {
    for (stuff::iterator i = m->begin(); i != m->end(); i++) {
        std::cout << i->first << " " << i->second << '\n';
    }
}

typedef struct header {
    std::string field;
    std::string value;
} header;

#include <chrono>
#include <iostream>
#include <map>
#include <map>
extern char **environ;

int main(int argc, char **argv, char **env) {
    int pid = fork();
    if (pid == 0) {
        chdir("www/");
        char *args[] = {(char *)"/opt/homebrew/bin/php-cgi", (char *)"./php.php", NULL};
        if (execve(args[0], args, environ) == -1) {
            std::cout << strerror(errno);
        }
    }
    waitpid(pid, NULL, 0);

    // for (int i = 0; i < 2000; i++) {
    //     std::cout << i << '\n';
    //     int pid = fork();
    //     if (pid == -1) {
    //         std::cerr << "fork: " << strerror(errno) << '\n';
    //         return 1;
    //     }
    //     if (pid == 0) {
    //         return 1;
    //     }
    //     if (kill(pid, SIGTERM) == -1) {
    //         std::cerr << "kill: " << strerror(errno) << '\n';
    //         return 1;
    //     }
    //     waitpid(pid, NULL, 0);
    // }
    return 0;
}
