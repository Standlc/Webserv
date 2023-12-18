#ifndef STATUS_COMMENTS_HPP
#define STATUS_COMMENTS_HPP

#include "../webserv.hpp"

class StatusComments {
   private:
    static std::map<int, String> _comments;

   public:
    StatusComments() {
    }

    static void init() {
        _comments[0] = "";
        _comments[200] = "OK";
        _comments[201] = "Created";
        _comments[301] = "Moved Permanently";
        _comments[302] = "Found";
        _comments[303] = "See Other";
        _comments[304] = "Not Modified";
        _comments[307] = "Temporary Redirect";
        _comments[308] = "Permanent Redirect";
        _comments[400] = "Bad Request";
        _comments[403] = "Forbidden Resource";
        _comments[404] = "Resource Not Found";
        _comments[405] = "Method Not Allowed";
        // _comments[408] = "Request Timeout";
        _comments[409] = "Conflict";
        _comments[411] = "Length Required";
        _comments[413] = "Payload Too Large";
        _comments[415] = "Unsupported Media Type";
        _comments[421] = "Misdirected Request";
        _comments[500] = "Internal Server Error";
        _comments[501] = "Not Implemented";
        _comments[502] = "Bad Gateway";
        _comments[504] = "Gateway Timeout";
    }

    static const String &get(int status) {
        if (_comments.find(status) == _comments.end()) {
            return _comments[0];
        }
        return _comments[status];
    }
};

#endif