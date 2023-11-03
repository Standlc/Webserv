#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#define TEST_HTML "HTTP/1.1 200 OK\nContent-Type: text/html\r\n\r\n<!DOCTYPE html>\n<html>\n<head><script src='myscripts.js'></script><link rel='stylesheet' type='text/css' href='styles.css' />\n<title>HTML page for Webserv</title>\n</head>\n<body>\n<h1>Hello, World!</h1>\n<p>This is a sample HTML page.</p>\n</body>\n</html>\n"
#define BUF_SIZE 2048
// #define BUF_SIZE 10

#define LINE_TERM "\r\n"

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

struct addrinfo *getOwnAddressInfo(const char *port);
int checkFileAccess(std::string path);
int createBindedNonBlockingSocket(struct addrinfo *addrInfo);
int bindSocket(int socketFd, struct addrinfo *addrInfo);
int listenToSocket(int socketFd, std::string port);
int getFileContent(std::string path, std::string &buf);
std::string getFileExtension(std::string fileName);
std::string getSocketPort(int socket);
int isDirectory(std::string path);
void compressSlashes(std::string &str);
 
class LocationBlock;
class ServerBlock;
class Server;
class HttpRequest;
class HttpResponse;
class MediaTypes;

typedef void (*pathHandlerType)(LocationBlock &block, HttpRequest &req, HttpResponse &res);
typedef std::map<std::string, pathHandlerType> methods;

#include "StatusComments.hpp"
#include "MediaTypes.hpp"
#include "HttpResponse.hpp"
#include "HttpRequest.hpp"
#include "Block.hpp"
#include "ServerBlock.hpp"
#include "LocationBlock.hpp"
#include "Server.hpp"

#endif