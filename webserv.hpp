#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#define BUF_SIZE 8000

#define CRLF "\r\n"
#define CRLF_CRLF "\r\n\r\n"
#define TIMEOUT 60

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

#define HTTP_VERSION "HTTP/1.0"

#define CYAN "\033[1;36m"
#define GRAY "\033[1;30m"
#define PURPLE "\033[1;34m"
#define YELLOW "\033[1;93m"
#define GREEN "\033[1;32m"
#define RED "\033[1;31m"
#define BOLD "\033[1m"
#define WHITE "\033[0m"
#define DIM_RED "\033[2;31m"

void debug(std::string title, std::string arg, std::string color);
void debugErr(const std::string &title, const char *err);
void exitProgram(Server &server);

struct addrinfo *getOwnAddressInfo(const char *port);
int checkPathAccess(std::string path);
int createBindedSocket(struct addrinfo *addrInfo);
int bindSocket(int socketFd, struct addrinfo *addrInfo);
int listenToSocket(int socketFd, std::string port);
int getFileContent(std::string path, std::string &buf);
std::string getFileExtension(std::string fileName);
std::string getSocketPort(int socket);
int isDirectory(std::string path);
void compressSlashes(std::string &str);
std::vector<std::string> split(const std::string &str, const std::string &sep);
std::string generateDirectoryListingPage(std::string dir, std::string reqUrl, struct dirent *entry, DIR *dirStream);
void readNextEntry(DIR *dirStream, struct dirent **entry);
bool startsWith(const std::string &str, const std::string &startWith, size_t fromPos = 0);
class Server;
class PollFd;
class ClientPollFd;
class CgiReadPollFd;
class CgiWritePollFd;

int handleNewConnection(Server &server, PollFd *listen);
int checkTimeout(time_t time);

int sendResponseToClient(Server &server, ClientPollFd *client);
int executeClientRequest(Server &server, ClientPollFd *client);
int readClientRequest(Server &server, ClientPollFd *client);
int timeoutClient(Server &server, ClientPollFd *client);

int waitCgiProcessEnd(Server &server, ClientPollFd *client);
int readCgiResponseFromPipe(Server &server, CgiReadPollFd *cgi);
int sendCgiRequest(Server &server, CgiWritePollFd *cgi);
int waitEmptyCgiPipe(Server &server, ClientPollFd *client);

std::string g_conf_path;
char **g_env;

#endif