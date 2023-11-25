#include "../../Block.hpp"

void LocationBlock::deleteMethod(HttpRequest &req, HttpResponse &res) {
    String resourcePath = this->getResourcePath(req.url().path);
    debug("deleting", resourcePath, YELLOW);

    if (checkPathAccess(resourcePath) == 404) {
        throw 404;
    }
    if (isDirectory(resourcePath)) {
        throw 403;
    }
    if (std::remove(&resourcePath[0]) != 0) {
        throw 500;
    }

    res.loadFile(200, "defaultPages/delete_success.html");
}