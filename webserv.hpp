#ifndef WEBSERV_HPP
#define WEBSERV_HPP

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

#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#define DEBUG true
#define WEBSERV_V "webserv/1.0"
#define HTTP_VERSION "HTTP/1.1"
#define BUF_SIZE 8000
#define CGI_SOCK_SIZE 6000000
#define TIMEOUT 60
#define CGI_TIMEOUT 10
#define CRLF "\r\n"
#define CRLF_CRLF "\r\n\r\n"
#define NPOS String::npos

#define CYAN "\033[1;36m"
#define GRAY "\033[1;30m"
#define PURPLE "\033[1;34m"
#define YELLOW "\033[1;93m"
#define GREEN "\033[1;32m"
#define RED "\033[3;31m"
#define BOLD "\033[1m"
#define WHITE "\033[0m"
#define DIM_RED "\033[2;31m"

typedef std::string String;
typedef std::unordered_map<String, String> unorderedStringMap;
typedef std::unordered_multimap<String, String> unorderedStringMultiMap;

class Server;
class Block;
class LocationBlock;
class ServerBlock;
class ServerStream;
class HttpRequest;
class HttpResponse;
class PollFd;
class CgiPoll;
class ClientPoll;

void closeOpenFd(int &fd);
void debug(const String &title, const String &arg, const String &color);
void debugErr(const String &title, const char *err);
void exitProgram(Server *server, int exitCode);
String parsePathFileName(const String &path);
String parseFileDirectory(const String &filePath);
void trySetenv(const String &name, const String &value);
void tryUnsetenv(const String &name);
int checkPathAccess(const String &path);
int getFileContent(const String &path, String &buf);
String getFileExtension(const String &fileName);
int isDirectory(const String &path);
void compressSlashes(String &str);
std::vector<String> split(const String &str, const String &sep);
String generateDirectoryListingPage(const String &dir, String reqUrl, struct dirent *entry, DIR *dirStream);
void readNextEntry(DIR *dirStream, struct dirent **entry);
bool startsWith(const String &str, const String &startWith, size_t fromPos = 0);
bool isUnkownMethod(const String &method);
std::vector<String> split(const String &str, const String &sep);
size_t countBackSpaces(const String &str, const String &sep, size_t end);
size_t countFrontSpaces(const String &str, const String &sep, size_t pos);
size_t findEnd(const String &str, const String &sep, size_t pos);
void compressSlashes(String &str);
void uriDecode(String &url, const String &find, const String &replace);
void percentDecode(String &url);
bool isReadable(struct pollfd &pollEl);
bool isWritable(struct pollfd &pollEl);
int checkPollError(struct pollfd &pollEl, int error);
int checkPollErrors(struct pollfd &pollEl);

typedef int (*clientPollHandlerType)(Server &, ClientPoll *);
typedef int (*pollFdHandlerType)(Server &, PollFd *);
typedef int (*CgiPollHandlerType)(Server &, CgiPoll *);

extern String g_conf_path;
extern char **environ;

#endif