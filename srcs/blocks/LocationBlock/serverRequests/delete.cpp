#include "../../Block.hpp"

void LocationBlock::deleteMethod(HttpRequest &req, HttpResponse &res) {
    String resourcePath = this->getReqResourcePath(req);
    // String resourcePath = this->getResourcePath(req.url().path);

    throwIf(checkPathAccess(resourcePath) == 404, 404);
    throwIf(isDirectory(resourcePath), 403);
    throwIf(std::remove(&resourcePath[0]) != 0, 500);

    res.set(200, ".txt", "File was deleted.");
}