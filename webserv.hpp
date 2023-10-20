#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#define TEST_HTML "HTTP/1.1 200 OK\nContent-Type: text/html\r\n\r\n<!DOCTYPE html>\n<html>\n<head><script src='myscripts.js'></script><link rel='stylesheet' type='text/css' href='styles.css' />\n<title>HTML page for Webserv</title>\n</head>\n<body>\n<h1>Hello, World!</h1>\n<p>This is a sample HTML page.</p>\n</body>\n</html>\n"
#define BUF_SIZE 2048
// #define BUF_SIZE 10

#define LINE_TERM "\r\n"

#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <algorithm>

struct addrinfo *getOwnAddressInfo(const char *port);
std::string checkFileAccess(std::string path);
int createBindedNonBlockingSocket(struct addrinfo *addrInfo);
int bindSocket(int socketFd, struct addrinfo *addrInfo);
int listenToSocket(int socketFd, std::string port);
int getFileContent(std::string path, std::string &buf);
std::string getFileExtension(std::string fileName);
std::string getSocketPort(int socket);

class ServerBlock;
class Server;
class HttpRequest;
class HttpResponse;
class MediaTypes;

typedef std::string (*methodHandler)(ServerBlock &block, HttpRequest &req, HttpResponse &res);
typedef std::map<std::string, methodHandler> methods;

#include "MediaTypes.hpp"
#include "HttpResponse.hpp"
#include "HttpRequest.hpp"
#include "ServerBlock.hpp"
// #include "ServerConfig.hpp"
#include "Server.hpp"

#endif