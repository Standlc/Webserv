#ifndef STATUS_COMMENTS_HPP
#define STATUS_COMMENTS_HPP

#include "webserv.hpp"

class StatusComments
{
private:
	static std::map<int, std::string> _comments;
	static bool _isInit;

public:
	StatusComments()
	{
	}

	static void init()
	{
		if (_isInit)
			return;

		_isInit = true;
		_comments[200] = "OK";
		_comments[301] = "Moved Permanently";
		_comments[303] = "See Other";
		_comments[400] = "Bad Request";
		_comments[403] = "Forbidden Resource";
		_comments[404] = "Resource Not Found";
		_comments[405] = "Method Not Allowed";
		_comments[409] = "Conflict";
		_comments[413] = "Payload Too Large";
		_comments[500] = "Internal Server Error";
		_comments[501] = "Not Implemented";
	}

	static std::string get(int status)
	{
		init();
		if (!_comments.count(status))
			return "undefined";
		return _comments[status];
	}
};

bool StatusComments::_isInit = false;
std::map<int, std::string> StatusComments::_comments;

#endif