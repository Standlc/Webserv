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
// char *const args[] = {(char *)"/usr/bin/python3", (char *)"./www/cgi/hello.py", NULL};

int main() {
    // int pipes[2];
    // int pipesRead[2];
    // pipe(pipes);
    // pipe(pipesRead);

    // int pid = fork();
    // if (pid == 0) {
    //     dup2(pipesRead[1], 1);
    //     close(pipesRead[0]);
    //     close(pipesRead[1]);

    //     dup2(pipes[0], 0);
    //     close(pipes[0]);
    //     close(pipes[1]);
    //     char *const args[] = {(char *)"/usr/bin/python3", (char *)"./www/cgi/hello.py", NULL};
    //     if (execve(*args, args, env) == -1) {
    //         std::cerr << strerror(errno) << "\n";
    //     }

    //     // char buf[100 + 1];
    //     // std::cerr << "reading" << '\n';
    //     // buf[read(0, buf, 100)] = '\0';
    //     // std::cerr << buf << '\n';

    //     // buf[read(0, buf, 100)] = '\0';
    //     // std::cerr << buf << '\n';

    //     // write(1, "hey!", 4);
    //     // std::cerr << "done writting" << '\n';
    //     return 0;
    // }

    // std::cerr << "sending" << '\n';
    // write(pipes[1], "hello", 5);
    // close(pipes[0]);
    // close(pipes[1]);
    // waitpid(pid, NULL, 0);

    // std::cout << "process has finished\n";
    // char buf[100 + 1];
    // buf[read(pipesRead[0], buf, 100)] = '\0';
    // std::cerr << buf << "\n";
    // close(pipesRead[1]);
    // close(pipesRead[0]);
    // return 0;

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