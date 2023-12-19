#include "../Block.hpp"

LocationBlock::LocationBlock() : Block() {
    _isExact = false;
    _allowedMethods.push_back("GET");
    _serverMethodshandlers["GET"] = &LocationBlock::getMethod;
    _serverMethodshandlers["POST"] = &LocationBlock::postMethod;
    _serverMethodshandlers["DELETE"] = &LocationBlock::deleteMethod;
    _requestHandler = &LocationBlock::serverMethodHandler;
    _proxyPass = NULL;
    _serverBlock = NULL;
};

LocationBlock::LocationBlock(ServerBlock &serverBlock) : Block(serverBlock), _serverBlock(new ServerBlock(serverBlock)) {
    _isExact = false;
    _allowedMethods.push_back("GET");
    _serverMethodshandlers["GET"] = &LocationBlock::getMethod;
    _serverMethodshandlers["POST"] = &LocationBlock::postMethod;
    _serverMethodshandlers["DELETE"] = &LocationBlock::deleteMethod;
    _requestHandler = &LocationBlock::serverMethodHandler;
    _proxyPass = NULL;
};

LocationBlock::~LocationBlock() {
    delete _proxyPass;
    delete _serverBlock;
}

LocationBlock::LocationBlock(const LocationBlock &b) {
    _proxyPass = NULL;
    _serverBlock = NULL;
    *this = b;
}

ServerBlock *LocationBlock::serverBlock() {
    return _serverBlock;
}

LocationBlock &LocationBlock::operator=(const LocationBlock &b) {
    Block::operator=(b);
    _serverMethodshandlers = b._serverMethodshandlers;
    _requestHandler = b._requestHandler;
    _allowedMethods = b._allowedMethods;
    _path = b._path;
    _isExact = b._isExact;

    delete _proxyPass;
    if (b._proxyPass) {
        _proxyPass = new ProxyUrl(*b._proxyPass);
    } else {
        _proxyPass = NULL;
    }
    _redirection = b._redirection;
    _fallBack = b._fallBack;

    delete _serverBlock;
    if (b._serverBlock) {
        _serverBlock = new ServerBlock(*b._serverBlock);
    } else {
        _serverBlock = NULL;
    }
    return *this;
}

String LocationBlock::getReqPathInfo(HttpRequest &req) {
    size_t pathSize = _path.size();
    String pathInfo;
    if (pathSize >= req.url().path.size()) {
        pathInfo = "/";
    } else {
        pathInfo = req.url().path.substr(pathSize);
    }
    return pathInfo[0] == '/' ? &pathInfo[1] : pathInfo;
}

void LocationBlock::setPath(const String &path, bool isExact) {
    if (path[0] != '/') {
        _path = "/" + path;
    }
    _path = path;
    compressSlashes(_path);
    _isExact = isExact;
}

void LocationBlock::setRedirection(int statusCode, String redirectionUrl) {
    _redirection.statusCode = statusCode;
    _redirection.url = redirectionUrl;
    _requestHandler = &LocationBlock::redirectionHandler;
}

void LocationBlock::setProxyPass(const String &proxyPass) {
    _proxyPass = new ProxyUrl(proxyPass);
    _requestHandler = &LocationBlock::proxyHandler;
}

void LocationBlock::setAllowedMethods(std::vector<String> methods) {
    _allowedMethods.clear();
    _allowedMethods.resize(0);
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

// ServerBlock &LocationBlock::serverBlock() {
//     return _serverBlock;
// }

bool LocationBlock::exceedsReqMaxSize(size_t size) {
    if (_reqBodyMaxSize == NPOS) {
        return false;
    }
    return size > _reqBodyMaxSize;
}
