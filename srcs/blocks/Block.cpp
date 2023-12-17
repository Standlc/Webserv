#include "Block.hpp"

String g_conf_path;

Block::Block() : _index("index.html"), _autoIndex(false), _reqBodyMaxSize(1000000){};

Block::Block(const Block &b) {
    *this = b;
}

Block::~Block() {
}

Block &Block::operator=(const Block &b) {
    _root = b._root;
    _uploadRoot = b._uploadRoot;
    _index = b._index;
    _errorFiles = b._errorFiles;
    _cgiCommands = b._cgiCommands;
    _reqBodyMaxSize = b._reqBodyMaxSize;
    _autoIndex = b._autoIndex;
    _headers = b._headers;
    _sessionCookies = b._sessionCookies;
    _fallBack = b._fallBack;
    return *this;
}

void Block::loadErrPage(int statusCode, HttpResponse &res, HttpRequest &req) {
    if (this->hasErrorPage(statusCode) == false) {
        throw statusCode;
    }

    try {
        res.loadFile(statusCode, this->getResourcePath(req.url().path, _errorFiles[statusCode]));
    } catch (int loadFileError) {
        throw loadFileError == 500 ? 500 : statusCode;
    }
}

String Block::getResourcePath(const String &reqPath, const String &file) {
    if (file == "") {
        std::cout << "path => " << _root + reqPath << "\n";
        return _root + reqPath;
    }

    if (file[0] == '/') {
        std::cout << "path => " << _root + file << "\n";
        return _root + file;
    }

    if (lastChar(reqPath) == '/') {
        std::cout << "path => " << _root + reqPath + file << "\n";
        return _root + reqPath + file;
    } else {
        std::cout << "path => " << _root + parseFileDirectory(reqPath) + file << "\n";
        return _root + parseFileDirectory(reqPath) + file;
    }
    // std::cout << "path => " << _root + reqPath + file << "\n";
    // return _root + reqPath + file;
}

String Block::getUploadFilePath(const String &reqPath, const String &file) {
    String root = _root;
    if (_uploadRoot != "") {
        root = _uploadRoot;
    }

    if (file == "") {
        std::cout << "1path => " << root + reqPath << "\n";
        return root + reqPath;
    }

    if (file[0] == '/') {
        std::cout << "2path => " << root + file << "\n";
        return root + file;
    }

    if (lastChar(reqPath) == '/') {
        std::cout << "3path => " << root + reqPath + file << "\n";
        return root + reqPath + file;
    } else {
        std::cout << "4path => " << root + parseFileDirectory(reqPath) + file << "\n";
        return root + parseFileDirectory(reqPath) + file;
    }
}

bool Block::hasErrorPage(int statusCode) {
    return _errorFiles.find(statusCode) != _errorFiles.end();
}

void Block::addErrorPage(int statusCode, String pagePath) {
    _errorFiles[statusCode] = pagePath;
}

void Block::addCgiCommand(String extension, String absoluteCommandPath) {
    _cgiCommands[extension] = absoluteCommandPath;
}

void Block::setRoot(String root) {
    if (root[0] == '/') {
        _root = root;
    } else {
        _root = g_conf_path + root;
    }
}

void Block::setUploadRoot(String dir) {
    if (dir[0] == '/') {
        _uploadRoot = dir;
    } else {
        _uploadRoot = g_conf_path + dir;
    }
}

void Block::setIndex(String index) {
    _index = index;
}

void Block::setFallBack(String file) {
    _fallBack = file;
}

void Block::setMaxBodySize(size_t size) {
    _reqBodyMaxSize = size;
}

void Block::setAutoIndex(bool isOn) {
    _autoIndex = isOn;
}

void Block::addHeader(String key, String value) {
    _headers.add(key, value);
}

void Block::addSessionCookie(String name) {
    if (name == "") {
        return;
    }
    if (std::find(_sessionCookies.begin(), _sessionCookies.end(), name) == _sessionCookies.end()) {
        _sessionCookies.push_back(name);
    }
}