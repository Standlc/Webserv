#include "../../Block.hpp"

void LocationBlock::getMethod(HttpRequest &req, HttpResponse &res) {
    String resourcePath = this->getResourcePath(req.url().path);

    int accessStatus = checkPathAccess(resourcePath);
    if (accessStatus != 200) {
        throw accessStatus;
    }

    if (!isDirectory(resourcePath)) {
        res.loadFile(200, resourcePath);
        return;
    }

    try {
        res.loadFile(200, this->getResourcePath(req.url().path, this->getIndex()));
    } catch (int status) {
        if (status == 500) {
            throw 500;
        }
        if (this->isAutoIndex() == false) {
            throw 403;
        }
        res.listDirectory(resourcePath, req.url().path);
    }
}
