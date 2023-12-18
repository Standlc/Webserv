#include "../../Block.hpp"

void LocationBlock::getMethod(HttpRequest &req, HttpResponse &res) {
    String pathInfo = this->getReqPathInfo(req);
    String resourcePath = this->getResourcePath(_path, pathInfo);
    int accessStatus = checkPathAccess(resourcePath);

    if (accessStatus != 200 && _fallBack != "") {
        resourcePath = this->getResourcePath(_path, _fallBack);
        accessStatus = checkPathAccess(resourcePath);
    }

    throwIf(accessStatus != 200, accessStatus);

    if (!isDirectory(resourcePath)) {
        res.loadFile(200, resourcePath);
        return;
    }

    try {
        res.loadFile(200, this->getResourcePath(_path, _index));
    } catch (int status) {
        throwIf(status == 500, 500);
        throwIf(_autoIndex == false, 403);
        res.listDirectory(resourcePath, req.url().path);
    }
}
