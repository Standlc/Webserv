#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#define BUF_SIZE 2048

#define LINE_TERM "\r\n"

#include <iostream>
#include <string>
#include <signal.h>
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

struct addrinfo *getOwnAddressInfo(const char *port);
int checkFileAccess(std::string path);
int createBindedSocket(struct addrinfo *addrInfo);
int bindSocket(int socketFd, struct addrinfo *addrInfo);
int listenToSocket(int socketFd, std::string port);
int getFileContent(std::string path, std::string &buf);
std::string getFileExtension(std::string fileName);
std::string getSocketPort(int socket);
int isDirectory(std::string path);
void compressSlashes(std::string &str);
 
// class LocationBlock;
// class ServerBlock;
// class Server;
// class HttpRequest;
// class HttpResponse;
// class MediaTypes;

// #include "RecvFd.hpp"
// #include "CgiResponse.hpp"
// #include "StatusComments.hpp"
// #include "MediaTypes.hpp"
// #include "HttpResponse.hpp"
// #include "HttpRequest.hpp"
// #include "PollElement.hpp"
// #include "Block.hpp"
// #include "Server.hpp"
// #include "ServerBlock.hpp"
// #include "LocationBlock.hpp"

#endif