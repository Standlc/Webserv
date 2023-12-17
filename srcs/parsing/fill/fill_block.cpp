#include "../parsing.hpp"

int fill_auto_index(const string &file, Block *location) {
    int AutoIndex = false;
    if (!strncmp(file.c_str(), "on", strlen("on")))
        AutoIndex = true;
    location->setAutoIndex(AutoIndex);
    return (file.find(';') + 1);
}

int fill_cgi_extensions(const string &file, Block *location) {
    int i = 1;

    while (file[i] != '}') {
        String extention = file.substr(i, file.find(':', i) - i);
        i = file.find(':', i);
        i++;
        String binarie = file.substr(i, file.find(';', i) - i);
        location->addCgiCommand(extention, binarie);
        while (file[i] != ';')
            i++;
        i++;
    }
    return (i + 1);
}

int fill_add_header(const string &file, Block *location) {
    int i = 1;

    while (file[i] != '}') {
        String key = file.substr(i, file.find(':', i) - i);
        i = file.find(':', i);
        i++;
        String value = file.substr(i, file.find(';', i) - i);
        location->addHeader(key, value);
        while (file[i] != ';')
            i++;
        i++;
    }
    return (i + 1);
}

int fill_body_max_size(const string &file, Block *location) {
    int i = 0;

    size_t number = atoi(file.c_str());
    while (isdigit(file[i]))
        i++;
    if (!strncmp(file.substr(i, file.find(';') - i).c_str(), "MB", strlen("MB"))) {
        number *= 1000000;
        i += 2;
    }
    if (!strncmp(file.substr(i, file.find(';') - i).c_str(), "KB", strlen("KB"))) {
        number *= 1000;
        i += 2;
    }
    location->setMaxBodySize(number);
    return (i + 1);
}

int fill_fallback(const string &file, Block *server) {
    server->setFallBack(file.substr(0, file.find(';')));
    return (file.find(';') + 1);
}

int fill_cookie(const string &file, Block *server) {
    server->addSessionCookie(file.substr(0, file.find(';')));
    return (file.find(';') + 1);
}

int fill_index(const string &file, Block *server) {
    server->setIndex(file.substr(0, file.find(';')));
    return (file.find(';') + 1);
}

int fill_upload_root(const string &file, Block *server) {
    server->setUploadRoot(file.substr(0, file.find(';')));
    return (file.find(';') + 1);
}

int fill_root(const string &file, Block *server) {
    server->setRoot(file.substr(0, file.find(';')));
    return (file.find(';') + 1);
}

int fill_error_pages(const string &file, Block *server) {
    int i = 1;
    int number = 0;

    while (file[i] != '}') {
        number = atoi(&file[i]);
        while (file[i] != ':')
            i++;
        i++;
        String file_error = file.substr(i, file.find(';', i) - i);
        server->addErrorPage(number, file_error);
        while (file[i] != ';')
            i++;
        i++;
    }
    return (i + 1);
}
