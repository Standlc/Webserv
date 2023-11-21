#include "../PollEvents/PollFd.hpp"
#include "../Server.hpp"
#include "../webserv.hpp"

void closeOpenFd(int &fd) {
    if (fd != -1) {
        close(fd);
        fd = -1;
    }
}

bool isReadable(struct pollfd &pollEl) {
    return (pollEl.revents & POLLIN);
}

bool isWritable(struct pollfd &pollEl) {
    return (pollEl.revents & POLLOUT);
}

int checkPollError(struct pollfd &pollEl, int error) {
    if ((pollEl.revents & error)) {
        debug("socket event", std::to_string(error) + ", on fd: " + std::to_string(pollEl.fd), RED);
        return error;
    }
    return 0;
}

int checkPollErrors(struct pollfd &pollEl) {
    if (checkPollError(pollEl, POLLERR) ||
        checkPollError(pollEl, POLLHUP) ||
        checkPollError(pollEl, POLLNVAL)) {
        return -1;
    }
    return 0;
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

int getFileContent(const String &path, String &buf) {
    std::ifstream file(path);
    if (!file) {
        return 1;
    }

    std::stringstream fileContent;
    fileContent << file.rdbuf();
    buf = fileContent.str();
    file.close();
    return 0;
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

void exitProgram(Server *server, int exitCode) {
    delete server;
    debugErr("EXITING", "");
    exit(exitCode);
}

bool isUnkownMethod(const String &method) {
    return method != "GET" && method != "POST" && method != "DELETE";
}

void debugErr(const String &title, const char *err) {
    std::cerr << RED << title << ": " << err << "\n"
              << WHITE;
}

void debug(const String &title, const String &arg, const String &color) {
    if (!DEBUG) {
        return;
    }
    std::cout << color << title;
    if (arg != "") {
        std::cout << ": " << arg;
    }
    std::cout << "\n"
              << WHITE;
}

size_t countBackSpaces(const String &str, const String &sep, size_t end) {
    if (end == 0) {
        return 0;
    }

    String spaces = " \t";
    end--;
    int nSpaces = 0;
    char c = str[end];

    while (spaces.find(c) != NPOS && sep.find(c) == NPOS && c != '\"') {
        nSpaces++;
        if (end - nSpaces == 0) {
            break;
        }
        c = str[end - nSpaces];
    }
    return nSpaces;
}

size_t countFrontSpaces(const String &str, const String &sep, size_t pos) {
    String spaces = " \t";
    size_t strSize = str.size();
    int nSpaces = 0;
    char c = str[pos];

    while (spaces.find(c) != NPOS && sep.find(c) == NPOS && c != '\"') {
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

std::vector<String> split(const String &str, const String &sep) {
    std::vector<String> split;
    size_t strLen = str.length();
    size_t start = 0;
    size_t end = -1;

    while (end < strLen || end == NPOS) {
        start = end + 1;
        end = findEnd(str, sep, start);
        end = (end != NPOS) ? end : strLen;

        start += countFrontSpaces(str, sep, start);
        size_t backSpaces = countBackSpaces(str, sep, end);

        size_t size = (end - backSpaces < start) ? 0 : (end - backSpaces - start);
        split.push_back(str.substr(start, size));
    }
    return split;
}