#ifndef MEDIA_TYPES_HPP
#define MEDIA_TYPES_HPP

#include "../webserv.hpp"

class MediaTypes {
   private:
    static std::map<String, String> _types;

   public:
    MediaTypes() {
    }

    static void init() {
        _types["undefined"] = "application/octet-stream";
        _types[".txt"] = "text/plain";
        _types[".html"] = "text/html";
        _types[".htm"] = "text/html";
        _types[".css"] = "text/css";
        _types[".js"] = "text/javascript";
        _types[".json"] = "application/json";
        _types[".jpeg"] = "image/jpeg";
        _types[".jpg"] = "image/jpeg";
        _types[".png"] = "image/png";
        _types[".gif"] = "image/gif";
        _types[".svg"] = "image/svg+xml";
        _types[".bmp"] = "image/bmp";
        _types[".webp"] = "image/webp";
        _types[".mp3"] = "audio/mpeg";
        _types[".wav"] = "audio/wav";
        _types[".ogg"] = "audio/ogg";
        _types[".mp4"] = "video/mp4";
        _types[".webm"] = "video/webm";
        _types[".ogv"] = "video/ogg";
        _types[".pdf"] = "application/pdf";
        _types[".zip"] = "application/zip";
        _types[".csv"] = "text/csv";
        _types[".xml"] = "application/xml";
        _types[".woff"] = "application/font-woff";
        _types[".ttf"] = "application/font-ttf";
        _types[".eot"] = "application/vnd.ms-fontobject";
        _types[".otf"] = "application/font-sfnt";
        _types[".doc"] = "application/msword";
        _types[".docx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
        _types[".xls"] = "application/vnd.ms-excel";
        _types[".xlsx"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
        _types[".ppt"] = "application/vnd.ms-powerpoint";
        _types[".pptx"] = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
    }

    static const String &getType(String filePath) {
        String fileExtension = getFileExtension(filePath);
        if (_types.find(fileExtension) == _types.end()) {
            return _types["undefined"];
        }
        return _types[fileExtension];
    }
};

#endif