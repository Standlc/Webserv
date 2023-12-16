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
    // std::cout << "error_page:" << statusCode << ':' << pagePath << "-\n";
    _errorFiles[statusCode] = pagePath;
}

void Block::addCgiCommand(String extension, String absoluteCommandPath) {
    // std::cout << "cgi:" << extension << ':' << absoluteCommandPath << "-\n";
    _cgiCommands[extension] = absoluteCommandPath;
}

void Block::setRoot(String root) {
    // std::cout << "root:" << root << "-\n";
    if (root[0] == '/') {
        _root = root;
    } else {
        _root = g_conf_path + root;
    }
}

void Block::setUploadRoot(String dir) {
    // std::cout << "upload_root:" << dir << "-\n";
    if (dir[0] == '/') {
        _uploadRoot = dir;
    } else {
        _uploadRoot = g_conf_path + dir;
    }
}

void Block::setIndex(String index) {
    // std::cout << "index:" << index << "-\n";
    _index = index;
}

void Block::setFallBack(String file) {
    _fallBack = file;
}

void Block::setMaxBodySize(size_t size) {
    // std::cout << "bodysize:" << size << "-\n";
    _reqBodyMaxSize = size;
}

void Block::setAutoIndex(bool isOn) {
    // std::cout << "auto_index:" << isOn << "-\n";
    _autoIndex = isOn;
}

void Block::addHeader(String key, String value) {
    // std::cout << "add_header:" << key << ':' << value << "-\n";
    _headers.add(key, value);
}

void Block::addSessionCookie(String name) {
    // std::cout << "cookie:" << name << "-\n";
    if (name == "") {
        return;
    }
    if (std::find(_sessionCookies.begin(), _sessionCookies.end(), name) == _sessionCookies.end()) {
        _sessionCookies.push_back(name);
    }
}