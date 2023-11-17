#ifndef BLOCK_HPP
#define BLOCK_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "webserv.hpp"

extern std::string g_conf_path;

typedef struct Redirection {
    std::string url;
    int statusCode;
} Redirection;

class Block {
   protected:
    std::string _index;
    bool _autoIndex;
    std::string _root;
    std::string _uploadRoot;
    size_t _reqBodyMaxSize;
    Redirection _redirection;

    std::unordered_multimap<std::string, std::string> _headers;
    std::unordered_map<int, std::string> _errorFiles;
    std::unordered_map<std::string, std::string> _cgiExtensions;
    std::string _sessionCookieName;

   public:
    Block() : _index("index.html"), _autoIndex(false), _reqBodyMaxSize(1000000){};

    Block(const Block &b) {
        *this = b;
    }

    virtual ~Block() {
    }

    Block &operator=(const Block &b) {
        _root = b._root;
        _uploadRoot = b._uploadRoot;
        _index = b._index;
        _errorFiles = b._errorFiles;
        _cgiExtensions = b._cgiExtensions;
        _redirection = b._redirection;
        _reqBodyMaxSize = b._reqBodyMaxSize;
        _autoIndex = b._autoIndex;
        _headers = b._headers;
        _sessionCookieName = b._sessionCookieName;
        return *this;
    }

    void loadErrPage(int statusCode, HttpResponse &res, HttpRequest &req) {
        if (this->hasErrorPage(statusCode) == false) {
            throw statusCode;
        }

        try {
            res.loadFile(statusCode, this->getResourcePath(req, _errorFiles[statusCode]));
        } catch (int loadFileError) {
            throw loadFileError == 500 ? 500 : statusCode;
        }
    }

    std::string getResourcePath(HttpRequest &req, std::string file = "") {
        if (file == "") {
            return _root + req.getUrl();
        }

        if (file[0] == '/') {
            return _root + file;
        }

        const std::vector<std::string> &urlSplit = split(req.getUrl(), "/");

        std::string path = _root;
        for (size_t i = 0; i < urlSplit.size(); i++) {
            path += "/" + urlSplit[i];
        }
        return path + "/" + file;
    }

    std::string getUploadFilePath(const std::string &filename) {
        if (_uploadRoot != "") {
            return _uploadRoot + "/" + filename;
        }
        return _root + "/" + filename;
    }

    bool hasErrorPage(int statusCode) {
        return _errorFiles.find(statusCode) != _errorFiles.end();
    }

    void addErrorPage(int statusCode, std::string pagePath) {
        _errorFiles[statusCode] = pagePath;
    }

    void setRedirection(int statusCode, std::string redirectionUrl) {
        _redirection.statusCode = statusCode;
        _redirection.url = redirectionUrl;
    }

    void addCgiExtension(std::string extension, std::string compilerPath) {
        _cgiExtensions[extension] = compilerPath;
    }

    void setRoot(std::string root) {
        if (root[0] == '/') {
            _root = root;
        } else {
            _root = g_conf_path + root;
        }
    }

    void setUploadRoot(std::string dir) {
        if (dir[0] == '/') {
            _uploadRoot = dir;
        } else {
            _uploadRoot = g_conf_path + dir;
        }
    }

    void setIndex(std::string index) {
        _index = index;
    }

    void setMaxBodySize(size_t size) {
        if (size < 0) {
            return;
        }
        _reqBodyMaxSize = size;
    }

    void setAutoIndex(bool isOn) {
        _autoIndex = isOn;
    }

    void addHeader(std::string key, std::string value) {
        _headers.insert(std::make_pair(key, value));
    }

    void setSessionCookie(std::string name) {
        _sessionCookieName = name;
    }
};

#endif