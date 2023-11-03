#ifndef STATUS_COMMENTS_HPP
#define STATUS_COMMENTS_HPP

#include "webserv.hpp"

class StatusComments
{
private:
	static std::map<int, std::string> _comments;

public:
	StatusComments()
	{
	}

	static std::string get(int status)
	{
		if (!_comments.count(status))
			return "undefined";
		return _comments[status];
	}
};

std::map<int, std::string> StatusComments::_comments = {
	{200, "OK"},
	{400, "Bad Request"},
	{403, "Forbidden Resource"},
	{404, "Resource Not Found"},
	{405, "Method Not Allowed"},
	{409, "Conflict"},
	{500, "Internal Server Error"},
	{501, "Not Implemented"},
};

#endif