#ifndef LOCATION_BLOCK_HPP
#define LOCATION_BLOCK_HPP

#include "ServerBlock.hpp"
#include "webserv.hpp"

#define NO_FILE_OR_DIR 2

int isDirectory(std::string path)
{
	struct stat pathInfo;

	if (stat(&path[0], &pathInfo) == -1)
	{
		if (errno == ENOENT)
			return NO_FILE_OR_DIR;
		std::cerr << "stat: " << strerror(errno) << "\n";
		return -1;
	}

	// (s.st_mode & S_IFREG) FOR FILE
	return pathInfo.st_mode & S_IFDIR;
}

class LocationBlock : public Block
{
public:
	std::string path;
	bool isExact;
	bool autoIndex;
	std::map<std::string, pathHandlerType> handlers;

	std::string execute(HttpRequest &req, HttpResponse &res)
	{
		std::string statusCode = handlers[req.getHttpMethod()](*this, req, res);
		if (statusCode == "200")
			return "200";
		return this->returnErrPage(statusCode, res);
	}

	std::string getCompleteReqPath(std::string path, std::string index)
	{
		// IF RELATIVE ROOT PATH
		//	 PATH_TO_CONFIG_FILE + ROOT_PATH

		path = root + path;
		int isDir = isDirectory(path);
		if (isDir == NO_FILE_OR_DIR)
			return "404";
		if (isDir == -1)
			return "500";
		if (isDir)
			return path + "/" + index;
		return path;
	}

	void inheritServerBlock(Block &block)
	{
		if (indexFiles.size() == 0)
			indexFiles = block.indexFiles;
		if (errorFiles.size() == 0)
			errorFiles = block.errorFiles;
		if (root == "")
			root = block.root;
	}
};

#endif