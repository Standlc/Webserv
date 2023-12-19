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
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#define WEBSERV_V "webserv/1.0"
#define HTTP_VERSION "HTTP/1.1"
#define BUF_SIZE 8000
#define CGI_SOCK_SIZE 6000000
#define TIMEOUT 60
#define CGI_TIMEOUT 10
#define CRLF "\r\n"
#define CRLF_CRLF "\r\n\r\n"
#define NPOS String::npos

#define GRAY "\033[0;90m"
#define DIM_RED "\033[0;31m"
#define RED "\033[0;91m"
#define GREEN "\033[0;92m"
#define YELLOW "\033[0;93m"
#define BLUE "\033[0;94m"
#define PURPLE "\033[0;95m"
#define CYAN "\033[0;96m"
#define BOLD "\033[1m"
#define WHITE "\033[0m"

typedef std::string String;
typedef std::map<String, String> stringMap;

class Server;
class Block;
class LocationBlock;
class ServerBlock;
class ServerStream;
class HttpResponse;
class HttpRequest;
class CgiRequest;
class PollFd;
class CgiPoll;
class ClientPoll;
class ProxyPoll;
class Headers;

typedef struct header {
    std::string field;
    std::string value;
} header;

class SigintError : public std::exception {
    const char *what() const throw() {
        return "See ya!";
    }
};

void debugSending(const String &title, ServerStream &s, int socket, const String &color);
void debugParsingErr(ServerStream &s, int socket, const String &color);
void debugParsingSuccesss(ServerStream &s, int socket, const String &color);
void debugHttpMessage(const String &httpMessage, const String &color = WHITE);
void debug(const String &title, const String &arg = "", const String &color = WHITE);
void debugErr(const String &title, const char *err = NULL);

long hexToInt(const String &str);
long toInt(const String &str);
String toString(long n);
String lowercase(String str);
String capitalize(String str);
void handleSigint(int sig);
String getRealtivePathToFile(String path);
void throwIf(bool condition, int status);
size_t tryFind(const String &str, const String &find, size_t from = 0);
struct addrinfo *getServerAddressInfo(String serverIpAddress, String port);
void handleSigint(int sig);
void closeOpenFd(int &fd);
String parsePathFileName(const String &path);
String parseFileDirectory(const String &filePath);
void trySetenv(const String &name, const String &value);
void tryUnsetenv(const String &name);
int checkPathAccess(const String &path);
void getFileContent(const String &path, String &buf);
String getFileExtension(const String &fileName);
int isDirectory(const String &path);
void compressSlashes(String &str);
std::vector<String> split(const String &str, const String &sep, size_t maxSize = -1);
String generateDirectoryListingPage(const String &dir, String reqUrl, struct dirent *entry, DIR *dirStream);
void readNextEntry(DIR *dirStream, struct dirent **entry);
bool startsWith(const String &str, const String &startWith, size_t fromPos = 0);
bool isUnkownMethod(const String &method);
size_t countBackSpaces(const String &str, size_t end, const String &sep = "");
size_t countFrontSpaces(const String &str, size_t pos, const String &sep = "");
size_t findEnd(const String &str, const String &sep, size_t pos);
void compressSlashes(String &str);
void uriDecode(String &url, const String &find, const String &replace);
void percentDecode(String &url);
bool isReadable(struct pollfd &pollEl);
bool isWritable(struct pollfd &pollEl);
int checkPollError(struct pollfd &pollEl, int error);
int checkPollErrors(struct pollfd &pollEl);
char lastChar(const String &str);

typedef int (*clientPollHandlerType)(ClientPoll *);
typedef int (*pollFdHandlerType)(PollFd *);
typedef int (*CgiPollHandlerType)(CgiPoll *);

extern String g_conf_path;
extern char **environ;
extern int isDebug;

#endif