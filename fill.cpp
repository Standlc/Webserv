#include "parsing.hpp"

int redirect_priority;

int fill_redirect(const string &file, LocationBlock *location) {
    int i = 0;

    redirect_priority = 1;
    int number = atoi(file.c_str());
    i = file.find(',');
    i++;
    string url = file.substr(i, file.find(';') - i);
    location->setRedirection(number, url);
    return (file.find(';') + 1);
}

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

int fill_methods(const string &file, LocationBlock *location) {
    int i = 0;

    String methods = file.substr(0, file.find(';'));
    vector<String>  arrayMethods;
    while (methods[i]) {
        if (methods[i] == ',')
            i++;
        String method = methods.substr(i, methods.find(',', i) - i);
        i += method.size();
        arrayMethods.push_back(method);
    }
    arrayMethods.push_back("");
    location->setAllowedMethods(arrayMethods);
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

int fill_port(const string &file, ServerBlock *server) {
    string listen = file.substr(0, file.find(';'));
    string ipAddress;

    if (listen.find(':') != string::npos) {
        ipAddress = listen.substr(0, listen.find(':'));
        listen = listen.substr(listen.find(':') + 1);
    } else
        ipAddress = "";
    server->set(ipAddress, listen, true);
    return (file.find(';') + 1);
}

int fill_host_name(const string &file, ServerBlock *server) {
    int end = 0;
    int start = 0;

    while (file[start] != ';') {
        if (file[start] == ',')
            start++;
        server->addHostName(file.substr(start, file.find_first_of(",;", start) - start));
        start = file.find_first_of(",;", start);
    }
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

int fill_proxy_pass(const string &file, LocationBlock *server) {
    if (redirect_priority == 1)
        return (file.find(';') + 1);
    server->setProxyPass(file.substr(0, file.find(';')));
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

int found_location_data(const string &file, LocationBlock *location) {
    if (!strncmp(file.c_str(), "root:", strlen("root:")))
        return (fill_root(file.substr(5), location) + 5);
    if (!strncmp(file.c_str(), "index:", strlen("index:")))
        return (fill_index(file.substr(6), location) + 6);
    if (!strncmp(file.c_str(), "error_pages:", strlen("error_pages:")))
        return (fill_error_pages(file.substr(12), location) + 12);
    if (!strncmp(file.c_str(), "cgi_extensions:", strlen("cgi_extensions:")))
        return (fill_cgi_extensions(file.substr(15), location) + 15);
    if (!strncmp(file.c_str(), "auto_index:", strlen("auto_index:")))
        return (fill_auto_index(file.substr(11), location) + 11);
    if (!strncmp(file.c_str(), "add_headers:", strlen("add_headers:")))
        return (fill_add_header(file.substr(12), location) + 12);
    if (!strncmp(file.c_str(), "methods:", strlen("methods:")))
        return (fill_methods(file.substr(8), location) + 8);
    if (!strncmp(file.c_str(), "body_max_size:", strlen("body_max_size:")))
        return (fill_body_max_size(file.substr(14), location) + 14);
    if (!strncmp(file.c_str(), "proxy_pass:", strlen("proxy_pass:")))
        return (fill_proxy_pass(file.substr(11), location) + 11);
    if (!strncmp(file.c_str(), "cookie:", strlen("cookie:")))
        return (fill_cookie(file.substr(7), location) + 7);
    if (!strncmp(file.c_str(), "upload_root:", strlen("upload_root:")))
        return (fill_upload_root(file.substr(12), location) + 12);
    if (!strncmp(file.c_str(), "redirect:", strlen("redirect:")))
        return (fill_redirect(file.substr(9), location) + 9);
    if (!strncmp(file.c_str(), "fallback:", strlen("fallback:")))
        return (fill_fallback(file.substr(9), location) + 9);
    return (1);
}

int fill_location(const string &file, ServerBlock *server) {
    int i = 0;
    LocationBlock *location;

    location = server->addLocation();
    bool exact = false;
    if (file[i] == '=') {
        exact = true;
        i++;
    }
    location->setPath(file.substr(i, file.find('{') - i), exact);
    while (file[i] != '{')
        i++;
    i++;
    while (file[i] != '}')
        i += found_location_data(&file[i], location);
    location = NULL;
    return (i + 1);
}

int found_data(string const &file, ServerBlock *server) {
    if (!strncmp(file.c_str(), "listen:", strlen("listen:")))
        return (fill_port(file.substr(7), server) + 7);
    if (!strncmp(file.c_str(), "host_names:", strlen("host_names:")))
        return (fill_host_name(file.substr(11), server) + 11);
    if (!strncmp(file.c_str(), "auto_index:", strlen("auto_index:")))
        return (fill_auto_index(file.substr(11), server) + 11);
    if (!strncmp(file.c_str(), "root:", strlen("root:")))
        return (fill_root(file.substr(5), server) + 5);
    if (!strncmp(file.c_str(), "index:", strlen("index:")))
        return (fill_index(file.substr(6), server) + 6);
    if (!strncmp(file.c_str(), "error_pages:", strlen("error_pages:")))
        return (fill_error_pages(file.substr(12), server) + 12);
    if (!strncmp(file.c_str(), "location:", strlen("location:")))
        return (fill_location(file.substr(9), server) + 9);
    if (!strncmp(file.c_str(), "body_max_size:", strlen("body_max_size:")))
        return (fill_body_max_size(file.substr(14), server) + 14);
    if (!strncmp(file.c_str(), "cookie:", strlen("cookie:")))
        return (fill_cookie(file.substr(7), server) + 7);
    if (!strncmp(file.c_str(), "upload_root:", strlen("upload_root:")))
        return (fill_upload_root(file.substr(12), server) + 12);
    if (!strncmp(file.c_str(), "cgi_extensions:", strlen("cgi_extensions:")))
        return (fill_cgi_extensions(file.substr(15), server) + 15);
    if (!strncmp(file.c_str(), "add_headers:", strlen("add_headers:")))
        return (fill_add_header(file.substr(12), server) + 12);
    if (!strncmp(file.c_str(), "fallback:", strlen("fallback:")))
        return (fill_fallback(file.substr(6), server) + 6);
    return (1);
}

bool delete_for_fill(char c) {
    return (c == '\n');
}

void    erase_duplicates(vector<String> &host_names)
{
    sort(host_names.begin(), host_names.end());
    host_names.erase(unique(host_names.begin(), host_names.end()), host_names.end());
}

bool same_host_names(vector<String> host_names1, vector<String> host_names2)
{
    cout << "je passe\n" << "hostnames1 : " << host_names1[0] << " " << "hostnames2 : " << host_names2[0] << '\n';
    erase_duplicates(host_names1);
    erase_duplicates(host_names2);
    host_names1.insert(host_names1.end(), host_names2.begin(), host_names2.end());
    if (std::adjacent_find(host_names1.begin(), host_names1.end()) != host_names1.end())
        return (true);
    return (false);
}

bool same_data(ServerBlock block1, ServerBlock block2)
{
    if (block1.getIpAddress() == block2.getIpAddress()
        && block1.getPort() == block2.getPort()
        && same_host_names(block1.getHostNames(), block2.getHostNames()))
        return (true);
    return (false);
}

bool    check_different_server(Server *server)
{
    int index = 0;
    int index_checker;
    // ServerBlock block;
    int size = server->getServerSize();

    while (index < size)
    {
        index_checker = -1;
        // block = server->getServerBlock(index);
        while (++index_checker < size)
            if (index_checker != index && same_data(server->getServerBlock(index), server->getServerBlock(index_checker)))
                return (false);
        index++;
    }
    return (true);
}

bool fill_data(string file, Server *server) {
    int start = 0;
    int index = 0;
    ServerBlock *block;

    file.erase(std::remove_if(file.begin(), file.end(), delete_for_fill), file.end());
    redirect_priority = 0;
    while (file[index]) {
        index = file.find("server{", start);
        if (index == string::npos)
            break ;
        block = server->addBlock();
        index += 7;
        start = index;
        while (file[index] && file[index] != '}') {
            index += found_data(file.substr(index), block);
        }
    }
    if (!check_different_server(server))
        return (false);
    return (true);
}