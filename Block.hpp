#ifndef BLOCK_HPP
#define BLOCK_HPP

#include "webserv.hpp"

typedef struct Redirection
{
	std::string url;
	int statusCode;
} Redirection;

class Block
{
public:
	std::string root;
	std::vector<std::string> indexFiles;
	std::map<int, std::string> errorFiles;
	Redirection redirection;

	void returnErrPage(int statusCode, HttpResponse &res)
	{
		if (this->hasErrorPage(statusCode) == false)
			throw statusCode;

		std::string errPagePath = root + errorFiles[statusCode];

		try
		{
			res.loadFile(statusCode, errPagePath);
		}
		catch(int status)
		{
			if (status == 500)
				throw 500;

			throw statusCode;
		}
	}

	bool hasErrorPage(int statusCode)
	{
		return errorFiles.find(statusCode) != errorFiles.end();
	}
};

#endif