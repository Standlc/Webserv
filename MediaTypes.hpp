#ifndef MEDIA_TYPES_HPP
#define MEDIA_TYPES_HPP

#include "webserv.hpp"

class MediaTypes
{
private:
	static std::map<std::string, std::string> _types;

public:
	MediaTypes()
	{
	}

	static std::string getType(std::string filePath)
	{
		if (!_types.count(getFileExtension(filePath)))
			return _types["undefined"];
		return _types[getFileExtension(filePath)];
	}
};

std::map<std::string, std::string> MediaTypes::_types = {
	{"undefined", "text/plain"},
	{".txt", "text/plain"},
	{".html", "text/html"},
	{".js", "text/javascript"},
	{".css", "text/css"},
};

#endif