#include "../PollEvents/PollFd.hpp"
#include "../Server.hpp"
#include "../webserv.hpp"

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
        debug("error", std::to_string(error) + ", on socket: " + std::to_string(pollEl.fd), RED);
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
    std::ifstream file(path);
    if (!file) {
        throw 500;
    }

    file.seekg(0, file.end);
    size_t len = file.tellg();
    file.seekg(0, file.beg);
    buf.resize(len, '\0');

    file.read(&buf[0], len);
    file.close();

    // std::ifstream file(path);
    // if (!file) {
    //     debugErr("Error while reading", &path[0]);
    //     throw 500;
    // }
    // std::stringstream fileContent;
    // fileContent << file.rdbuf();
    // buf = fileContent.str();
    // file.close();
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
    debug(name, value, YELLOW);
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

void debugMessageInfos(const String &title, int fd, size_t size, const String &color) {
    if (!DEBUG) {
        return;
    }
    debug("<< " + title, std::to_string(fd) + ", size: " + std::to_string(size) + " bytes", color);
}

void debugHttpMessage(const String &httpMessage, const String &color) {
    if (!DEBUG) {
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
    if (!DEBUG) {
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
    pos = str.find_first_of(sep + "'", pos);
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
    size_t strLen = str.length();
    size_t start = 0;
    size_t end = -1;
    size_t splitSize = 0;

    while ((end < strLen || end == NPOS) && splitSize < maxSize) {
        start = end + 1;
        end = findEnd(str, sep, start);
        end = (end != NPOS) ? end : strLen;

        start += countFrontSpaces(str, start, sep);
        size_t backSpaces = countBackSpaces(str, end, sep);

        size_t size = (end - backSpaces < start) ? 0 : (end - backSpaces - start);
        split.push_back(str.substr(start, size));
        splitSize++;
    }
    return split;
}