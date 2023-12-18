#include "../../Block.hpp"

String getFormBoundary(HttpRequest &req) {
    std::vector<String> values = split(req.getHeader("Content-Type"), ";");
    if (values.size() < 2 || values[0] != "multipart/form-data") {
        throw 400;
    }

    std::vector<String> boundary = split(values[1], "=");
    if (boundary.size() != 2 || boundary[0] != "boundary" || boundary[1] == "") {
        throw 400;
    }
    return boundary[1];
}

String getFormFileName(HttpRequest &req, size_t from, size_t to) {
    String formContentDisposition = req.findBodyHeader("Content-Disposition", from, to);
    std::vector<String> headerValues = split(formContentDisposition, ";");
    if (headerValues.size() < 3 || headerValues[0] != "form-data") {
        throw 400;
    }

    std::vector<String> filenameKey = split(headerValues[2], "=");
    if (filenameKey.size() != 2 || filenameKey[0] != "filename" || filenameKey[1] == "\"\"") {
        throw 400;
    }

    size_t quotesSize = 2;
    String filename = filenameKey[1].substr(1, filenameKey[1].size() - quotesSize);
    percentDecode(filename);
    return filename;
}

void createFile(const String &name, const char *data, size_t size) {
    std::ofstream file(&name[0], std::ios::binary);
    if (!file) {
        throw 500;
    }

    debug("> creating file", name, YELLOW);
    debug("> size", toString(size), YELLOW);

    file.write(data, size);
    file.close();
    if (!file) {
        throw 500;
    }
}

bool findFormBoundaries(HttpRequest &req, const String &boundary, size_t &currFormPos, size_t &formHeadersEndPos, size_t &formBodyEndPos) {
    currFormPos = req.searchBody(boundary, currFormPos);
    if (currFormPos == NPOS) {
        throw 400;
    }
    currFormPos += boundary.size();
    if (startsWith(req.getBody(), "--", currFormPos)) {
        return true;
    }

    formBodyEndPos = req.searchBody(boundary, currFormPos);
    if (formBodyEndPos == NPOS) {
        throw 400;
    }

    formHeadersEndPos = req.searchBody(CRLF_CRLF, currFormPos + 1, formBodyEndPos);
    if (formHeadersEndPos == NPOS) {
        throw 400;
    }
    formHeadersEndPos += 4;
    return false;
}

void LocationBlock::postMethod(HttpRequest &req, HttpResponse &res) {
    String boundary = "--" + getFormBoundary(req);
    size_t currFormPos = 0;
    size_t formHeadersEndPos = 0;
    size_t formBodyEndPos = 0;

    while (true) {
        bool isEnd = findFormBoundaries(req, boundary, currFormPos, formHeadersEndPos, formBodyEndPos);
        if (isEnd) {
            break;
        }

        String filename = getFormFileName(req, currFormPos, formHeadersEndPos);
        String filePath = this->getUploadFilePath(filename);
        throwIf(checkPathAccess(filePath) != 404, 409);
        // if (filename.find('/') != NPOS) {
        //     throw 403;
        // }

        size_t fileSize = (formBodyEndPos - formHeadersEndPos) - 2;
        createFile(filePath, &req.getBody()[formHeadersEndPos], fileSize);
    }

    res.set(200, ".txt", "File successfully uploaded.");
}
