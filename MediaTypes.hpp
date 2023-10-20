#ifndef MEDIA_TYPES_HPP
#define MEDIA_TYPES_HPP

#include "webserv.hpp"

class MediaTypes
{
private:
	std::map<std::string, std::string> _types;

public:
	MediaTypes()
	{
		_types["undefined"] = "text/plain";
		_types[".txt"] = "text/plain";
		_types[".html"] = "text/html";
		_types[".js"] = "text/javascript";
		_types[".css"] = "text/css";
	}

	std::string getType(std::string filePath)
	{
		return _types[getFileExtension(filePath)];
	}
};

#endif