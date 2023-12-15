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

String Block::getResourcePath(const String &reqUrl, const String &file) {
    if (file == "") {
        return _root + reqUrl;
    }
    if (file[0] == '/') {
        return _root + file;
    }
    return _root + reqUrl + "/" + file;
}

String Block::getUploadFilePath(const String &filename) {
    if (_uploadRoot != "") {
        return _uploadRoot + "/" + filename;
    }
    return _root + "/" + filename;
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

void Block::setForceFile(String file) {
    _forceFile = file;
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