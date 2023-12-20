#include "../PollEvents/PollFd.hpp"
#include "../Server.hpp"
#include "../webserv.hpp"

void handleSigint(int sig) {
    (void)sig;
    throw "\nGracefully shutting down...";
}

long hexToInt(const String &str) {
    std::stringstream ss(str);
    long n;
    ss >> std::hex >> n;
    // if (ss.fail()) {
    //     throw std::invalid_argument("Invalid input");
    // }
    return n;
}

long toInt(const String &str) {
    std::stringstream ss(str);
    long n;
    ss >> n;
    // if (ss.fail()) {
    //     throw std::invalid_argument("Invalid input");
    // }
    return n;
}

String toString(long n) {
    std::stringstream ss;
    ss << n;
    return ss.str();
}

String getRealtivePathToFile(String path) {
    if (path == "") {
        return "";
    }

    int lastSlash = path.find_last_of("/");
    if (lastSlash == -1) {
        return "";
    }
    return path.substr(0, lastSlash + 1);
}

void throwIf(bool condition, int status) {
    if (condition) {
        throw status;
    }
}

void closeOpenFd(int &fd) {
    if (fd != -1) {
        close(fd);
        fd = -1;
    }
}

String capitalize(String str) {
    size_t size = str.size();
    for (size_t i = 0; i < size; i++) {
        if (std::isalpha(str[i]) && std::isupper(str[i])) {
            if (i == 0 || !std::isalpha(str[i - 1])) {
                str[i] = std::toupper(str[i]);
            }
        }
    }
    return str;
}

String lowercase(String str) {
    size_t size = str.size();
    for (size_t i = 0; i < size; i++) {
        if (std::isupper(str[i])) {
            str[i] = std::tolower(str[i]);
        }
    }
    return str;
}

size_t tryFind(const String &str, const String &find, size_t from) {
    size_t pos = str.find(find, from);
    if (pos == NPOS) {
        throw 400;
    }
    return pos;
}

bool isReadable(struct pollfd &pollEl) {
    return (pollEl.revents & POLLIN) == POLLIN;
}

bool isWritable(struct pollfd &pollEl) {
    return (pollEl.revents & POLLOUT) == POLLOUT;
}

int checkPollError(struct pollfd &pollEl, int error) {
    if ((pollEl.revents & error) == error) {
        debug("error", toString(error) + ", on socket: " + toString(pollEl.fd), DIM_RED);
        return error;
    }
    return 0;
}

int checkPollErrors(struct pollfd &pollEl) {
    int error = 0;
    if (!error) {
        error = checkPollError(pollEl, POLLHUP);
    }
    if (!error) {
        error = checkPollError(pollEl, POLLNVAL);
    }
    if (!error) {
        error = checkPollError(pollEl, POLLERR);
    }
    return error;
}

char lastChar(const String &str) {
    size_t size = str.size();
    if (!size) {
        return str[0];
    }
    return str[size - 1];
}

void compressSlashes(String &str) {
    int startSlashPos = str.find_first_of("/");
    int endSlashPos = str.find_first_not_of("/", startSlashPos);

    while (startSlashPos != -1) {
        str.replace(startSlashPos, endSlashPos != -1 ? endSlashPos - startSlashPos : -1, "/");
        startSlashPos = str.find_first_of("/", startSlashPos + 1);
        endSlashPos = str.find_first_not_of("/", startSlashPos);
    }
}

void uriDecode(String &url, const String &find, const String &replace) {
    int pos = url.find(find);

    while (pos != -1) {
        url.replace(pos, 3, replace);
        pos = url.find(find, pos + 1);
    }
}

void percentDecode(String &url) {
    uriDecode(url, "%20", " ");
    uriDecode(url, "%21", "!");
    uriDecode(url, "%22", "\"");
    uriDecode(url, "%23", "#");
    uriDecode(url, "%24", "$");
    uriDecode(url, "%25", "%");
    uriDecode(url, "%26", "&");
    uriDecode(url, "%27", "\"");
    uriDecode(url, "%28", "(");
    uriDecode(url, "%29", ")");

    uriDecode(url, "%2A", "*");
    uriDecode(url, "%2B", "+");
    uriDecode(url, "%2C", ",");
    uriDecode(url, "%2F", "/");

    uriDecode(url, "%3A", ":");
    uriDecode(url, "%3B", ";");
    uriDecode(url, "%3D", "=");
    uriDecode(url, "%3F", "?");

    uriDecode(url, "%40", "@");

    uriDecode(url, "%5B", "[");
    uriDecode(url, "%5D", "]");
}

bool startsWith(const String &str, const String &str2, size_t startPos) {
    return str.compare(startPos, str2.length(), str2) == 0;
}

void readNextEntry(DIR *dirStream, struct dirent **entry) {
    *entry = readdir(dirStream);
    if (!*entry && errno != 0) {
        debugErr("readdir", strerror(errno));
        closedir(dirStream);
        throw 500;
    }
}

String getFileExtension(const String &fileName) {
    size_t lastSlashPos = fileName.find_last_of('/');
    size_t dotPos = fileName.find_last_of(".");

    if (dotPos == NPOS || (dotPos < lastSlashPos && lastSlashPos != NPOS)) {
        return "undefined";
    }
    return fileName.substr(dotPos);
}

void getFileContent(const String &path, String &buf) {
    std::ifstream file;
    file.open(&path[0]);
    if (!file) {
        throw 500;
    }

    file.seekg(0, file.end);
    size_t len = file.tellg();
    file.seekg(0, file.beg);
    buf.resize(len, '\0');

    file.read(&buf[0], len);
    file.close();
    if (!file) {
        throw 500;
    }
}

String parsePathFileName(const String &path) {
    size_t finalSlashPos = path.find_last_of('/');
    if (finalSlashPos == NPOS) {
        return path;
    }
    return path.substr(finalSlashPos + 1, -1);
}

String parseFileDirectory(const String &filePath) {
    size_t finalSlashPos = filePath.find_last_of('/');
    if (finalSlashPos == NPOS) {
        return filePath;
    }
    return filePath.substr(0, finalSlashPos + 1);
}

void tryUnsetenv(const String &name) {
    if (unsetenv(&name[0])) {
        debugErr("unsetenv", strerror(errno));
        throw 1;
    }
}

void trySetenv(const String &name, const String &value) {
    if (setenv(&name[0], &value[0], 1) == -1) {
        debugErr("setenv", strerror(errno));
        throw 1;
    }
}

int isDirectory(const String &path) {
    struct stat pathInfo;

    if (stat(&path[0], &pathInfo) == -1) {
        debugErr("stat", strerror(errno));
        throw 500;
    }
    return (pathInfo.st_mode & S_IFDIR) != 0;
}

int checkPathAccess(const String &path) {
    if (access(&path[0], F_OK) == -1) {
        return 404;
    }
    if (access(&path[0], R_OK) == -1) {
        return 403;
    }
    return 200;
}

bool isUnkownMethod(const String &method) {
    return method != "GET" && method != "POST" && method != "DELETE";
}

void debugErr(const String &title, const char *err) {
    std::cerr << RED << title;
    if (err) {
        std::cerr << ": " << err;
    }
    std::cerr << "\n"
              << WHITE;
}

void debugParsingErr(ServerStream &s, int socket, const String &color) {
    if (!isDebug) {
        return;
    }
    std::cout << color << "> caught syntax error, socket: " << socket
              << ", size: " << s.totalRead() << " bytes\n"
              << WHITE;
    debugHttpMessage(s.rawData(), color);
}

void debugParsingSuccesss(ServerStream &s, int socket, const String &color) {
    if (!isDebug) {
        return;
    }
    std::cout << color << "> successfully parsed, socket: " << socket
              << ", size: " << s.totalRead() << " bytes\n"
              << color;
    debugHttpMessage(s.rawData(), color);
}

void debugSending(const String &title, ServerStream &s, int socket, const String &color) {
    if (!isDebug) {
        return;
    }
    std::cout << color << "<< " << title << ", socket: " << socket
              << ", size: " << s.outputDataSize() << " bytes\n"
              << color;
    debugHttpMessage(s.outputData(), color);
}

void debugHttpMessage(const String &httpMessage, const String &color) {
    if (!isDebug) {
        return;
    }
    size_t len = httpMessage.size();
    std::cout << color << "[" << WHITE;
    for (size_t i = 0; i < len; i++) {
        if (startsWith(httpMessage, CRLF_CRLF, i) || startsWith(httpMessage, "\n\n", i)) {
            break;
        }
        std::putchar(httpMessage[i]);
    }
    std::cout << color << "]" << WHITE << "\n\n";
}

void debug(const String &title, const String &arg, const String &color) {
    if (!isDebug) {
        return;
    }
    std::cout << color << title;
    if (arg != "") {
        std::cout << ": " << arg;
    }
    std::cout << WHITE << "\n";
}

size_t countBackSpaces(const String &str, size_t end, const String &sep) {
    if (end == 0) {
        return 0;
    }

    end--;
    int nSpaces = 0;
    char c = str[end];

    while (std::isspace(c) && sep.find(c) == NPOS && c != '\"') {
        nSpaces++;
        if (end - nSpaces == 0) {
            break;
        }
        c = str[end - nSpaces];
    }
    return nSpaces;
}

size_t countFrontSpaces(const String &str, size_t pos, const String &sep) {
    size_t strSize = str.size();
    int nSpaces = 0;
    char c = str[pos];

    while (std::isspace(c) && sep.find(c) == NPOS && c != '\"') {
        nSpaces++;
        if (pos + nSpaces == strSize) {
            break;
        }
        c = str[pos + nSpaces];
    }
    return nSpaces;
}

size_t findEnd(const String &str, const String &sep, size_t pos) {
    pos = str.find_first_of(sep + "\"", pos);
    if (pos == NPOS) {
        return pos;
    }

    if (sep.find(str[pos]) != NPOS) {
        return pos;
    } else {
        size_t endPos = str.find_first_of('\"', pos + 1);
        if (endPos == NPOS) {
            return endPos;
        }
        return findEnd(str, sep, endPos + 1);
    }
}

std::vector<String> split(const String &str, const String &sep, size_t maxSize) {
    std::vector<String> split;
    size_t start = 0;
    size_t end = 0;
    size_t size = 0;
    size_t splitSize = 0;
    int strlen = str.length();

    int i = 0;
    while (i < strlen && splitSize < maxSize) {
        if (sep.find(str[i]) == NPOS) {
            start = i;
            start += countFrontSpaces(str, start);
            while (i < strlen && (sep.find(str[i]) == NPOS || str[i] == '\"')) {
                while (str[i] == '\"') {
                    i++;
                    while (i < strlen && sep.find(str[i]) == NPOS && str[i] != '\"') {
                        i++;
                    }
                    if (sep.find(str[i]) == NPOS) {
                        break;
                    }
                }
                i++;
            }

            if (i >= strlen) {
                size = -1;
            } else {
                end = i - countBackSpaces(str, i);
                size = (end <= start) ? 0 : (end - start);
            }
            if (size > 0) {
                String item = str.substr(start, size);
                split.push_back(item);
                splitSize++;
            }
        }
        i++;
    }
    return split;
}

String generateDirectoryListingPage(const String &dir, String reqUrl, struct dirent *entry, DIR *dirStream) {
    if (lastChar(reqUrl) != '/') {
        reqUrl += "/";
    }

    String page = "<!DOCTYPE html><html><head>";
    page += "<title>Index of " + reqUrl + "</title></head><body ";
    page += "><h1> Index of " + reqUrl + " </h1><hr><pre style='display: flex;flex-direction:column;'>";

    errno = 0;
    while (entry) {
        String entry_name = entry->d_name;
        if (entry_name != "." && checkPathAccess(dir) == 200) {
            if (isDirectory(dir + "/" + entry_name)) {
                entry_name += "/";
            }
            page += "<a href='" + reqUrl + entry_name + "'>" + entry_name + "</a>";
        }
        readNextEntry(dirStream, &entry);
    }

    page += "</pre><hr></body></html>";
    return page;
}
