#include "../../Block.hpp"

String getFormBoundary(HttpRequest &req) {
    std::vector<String> values = split(req.getHeader("Content-Type"), ";");
    if (values.size() < 2 || values[0] != "multipart/form-data") {
        debug("400: 4", "", YELLOW);
        throw 400;
    }

    std::vector<String> boundary = split(values[1], "=");
    if (boundary.size() != 2 || boundary[0] != "boundary" || boundary[1] == "") {
        debug("400: 5", "", YELLOW);
        throw 400;
    }
    return boundary[1];
}

String getFormFileName(HttpRequest &req, size_t from, size_t to) {
    std::vector<String> headerValues = split(req.findBodyHeader("Content-Disposition", from, to), ";");
    if (headerValues.size() < 3 || headerValues[0] != "form-data") {
        debug("400: 6", "", YELLOW);
        throw 400;
    }

    std::vector<String> filenameKey = split(headerValues[2], "=");
    if (filenameKey.size() != 2 || filenameKey[0] != "filename" || filenameKey[1] == "\"\"") {
        debug("400: 7", "", YELLOW);
        throw 400;
    }

    String filename = filenameKey[1].substr(1, filenameKey[1].size() - 2);
    percentDecode(filename);
    return filename;
}

void createFile(const String &name, const char *data, size_t size) {
    std::ofstream file(&name[0], std::ios::binary);
    if (!file) {
        throw 500;
    }

    debug("filename", name, CYAN);
    debug("file size", std::to_string(size), CYAN);
    file.write(data, size);
    file.close();
}

bool findFormBoundaries(HttpRequest &req, const String &boundary, size_t &currFormPos, size_t &formHeadersEndPos, size_t &formBodyEndPos) {
    currFormPos = req.searchBody(boundary, currFormPos);
    if (currFormPos == NPOS) {
        debug("400: 1", "", YELLOW);
        throw 400;
    }
    currFormPos += boundary.size();
    if (!startsWith(req.getBody(), "--", currFormPos)) {
        return true;
    }

    formBodyEndPos = req.searchBody(boundary, currFormPos);
    if (formBodyEndPos == NPOS) {
        debug("400: 3", "", YELLOW);
        throw 400;
    }

    formHeadersEndPos = req.searchBody(CRLF_CRLF, currFormPos + 1, formBodyEndPos);
    if (formHeadersEndPos == NPOS) {
        debug("400: 2", "", YELLOW);
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
    String filePath;

    while (true) {
        debug("BEGIN FORM", "", YELLOW);
        bool isEnd = findFormBoundaries(req, boundary, currFormPos, formHeadersEndPos, formBodyEndPos);
        if (isEnd) {
            debug("END OF FORM", "", YELLOW);
            break;
        }

        String filename = getFormFileName(req, currFormPos, formHeadersEndPos);
        filePath = this->getUploadFilePath(filename);
        if (checkPathAccess(filePath) != 404) {
            throw 409;
        }
        // if (filename.find('/') != NPOS) {
        //     throw 403;
        // }

        size_t fileSize = (formBodyEndPos - formHeadersEndPos) - 2;
        createFile(filePath, &req.getBody()[formHeadersEndPos], fileSize);
    }

    res.loadFile(200, filePath);
}
