#include "../../Block.hpp"

void LocationBlock::deleteMethod(HttpRequest &req, HttpResponse &res) {
    String pathInfo = this->getReqPathInfo(req);
    if (pathInfo == "" || pathInfo == "/") {
        throw 400;
    }

    String resourcePath = this->getResourcePath(_path, pathInfo);
    debug("path", resourcePath, YELLOW);

    throwIf(checkPathAccess(resourcePath) == 404, 404);
    throwIf(isDirectory(resourcePath), 403);
    throwIf(std::remove(&resourcePath[0]) != 0, 500);

    res.set(200, ".txt", "File was deleted.");
}