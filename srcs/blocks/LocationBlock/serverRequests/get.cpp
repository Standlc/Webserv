#include "../../Block.hpp"

void LocationBlock::getMethod(HttpRequest &req, HttpResponse &res) {
    String resourcePath = this->getResourcePath(req.url().path, _forceFile);

    int accessStatus = checkPathAccess(resourcePath);
    throwIf(accessStatus != 200, accessStatus);

    if (!isDirectory(resourcePath)) {
        res.loadFile(200, resourcePath);
        return;
    }

    try {
        res.loadFile(200, this->getResourcePath(req.url().path, _index));
    } catch (int status) {
        throwIf(status == 500, 500);
        throwIf(_autoIndex == false, 403);
        res.listDirectory(resourcePath, req.url().path);
    }
}
