#ifndef STATUS_COMMENTS_HPP
#define STATUS_COMMENTS_HPP

#include "webserv.hpp"

class StatusComments {
   private:
    static std::unordered_map<int, String> _comments;
    static bool _isInit;

   public:
    StatusComments() {
    }

    static void init() {
        if (_isInit) {
            return;
        }

        _isInit = true;
        _comments[0] = "undefined";
        _comments[200] = "OK";
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
        _comments[409] = "Conflict";
        _comments[411] = "Length Required";
        _comments[413] = "Payload Too Large";
        _comments[415] = "Unsupported Media Type";
        _comments[421] = "Misdirected Request";
        _comments[500] = "Internal Server Error";
        _comments[501] = "Not Implemented";
        _comments[502] = "Bad Gateway";
    }

    static const String &get(int status) {
        init();
        if (_comments.find(status) == _comments.end())
            return _comments[0];
        return _comments[status];
    }
};

#endif