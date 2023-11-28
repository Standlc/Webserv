#include "../Block.hpp"

void LocationBlock::setPath(const String &path, bool isExact) {
    if (path[0] != '/') {
        _path = "/" + path;
    }
    _path = path;
    compressSlashes(_path);
    _isExact = isExact;
}

LocationBlock &LocationBlock::addLocation() {
    return _serverBlock.addLocation(*this);
}

void LocationBlock::setRedirection(int statusCode, String redirectionUrl) {
    _redirection.statusCode = statusCode;
    _redirection.url = redirectionUrl;
    _requestHandler = &LocationBlock::proxyHandler;
}

void LocationBlock::setProxyPass(const String &proxyPass) {
    _proxyPass = new ProxyUrl(proxyPass);
    _requestHandler = &LocationBlock::proxyHandler;
}

void LocationBlock::setAllowedMethods(String methods[]) {
    for (int i = 0; methods[i] != ""; i++) {
        _allowedMethods.push_back(methods[i]);
    }
}

bool LocationBlock::isMethodAllowed(const String &httpMethod) {
    return std::find(_allowedMethods.begin(), _allowedMethods.end(), httpMethod) != _allowedMethods.end();
}

bool LocationBlock::handlesHttpMethod(const String &httpMethod) {
    return _serverMethodshandlers.find(httpMethod) != _serverMethodshandlers.end();
}

const String &LocationBlock::getPath() {
    return _path;
}

bool LocationBlock::isExactPath() {
    return _isExact;
}

const String &LocationBlock::getIndex() {
    return _index;
}

bool LocationBlock::isAutoIndex() {
    return _autoIndex;
}

ServerBlock &LocationBlock::serverBlock() {
    return _serverBlock;
}

bool LocationBlock::exceedsReqMaxSize(size_t size) {
    if (_reqBodyMaxSize == NPOS) {
        return false;
    }
    return size > _reqBodyMaxSize;
}
