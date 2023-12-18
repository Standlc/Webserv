#include "../parsing.hpp"

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
        return (fill_fallback(file.substr(9), server) + 9);
    return (1);
}