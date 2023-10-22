#ifndef BLOCK_HPP
#define BLOCK_HPP

#include "webserv.hpp"

class Block
{
public:
	std::string root;
	std::vector<std::string> indexFiles;
	std::map<int, std::string> errorFiles;

	int returnErrPage(int statusCode, HttpResponse &res)
	{
		if (errorFiles.find(statusCode) == errorFiles.end())
			return statusCode;

		std::string errPagePath = root + errorFiles[statusCode];
		int loadFileStatus = res.loadFile(statusCode, errPagePath, "error");

		if (loadFileStatus == 500)
			return 500;
		return loadFileStatus == 200 ? 200 : statusCode;
	}
};

#endif