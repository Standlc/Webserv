#ifndef LOCATION_BLOCK_HPP
#define LOCATION_BLOCK_HPP

#include "ServerBlock.hpp"
#include "webserv.hpp"

int isDirectory(std::string path)
{
	struct stat pathInfo;

	if (stat(&path[0], &pathInfo) == -1)
	{
		std::cerr << "stat: " << strerror(errno) << "\n";
		return -1;
	}

	// (s.st_mode & S_IFREG) FOR FILE
	return (pathInfo.st_mode & S_IFDIR) != 0;
}

class LocationBlock : public Block
{
public:
	std::string path;
	bool isExact;
	bool autoIndex;
	std::map<std::string, pathHandlerType> handlers;

	int execute(HttpRequest &req, HttpResponse &res)
	{
		if (redirection.url != "")
		{
			std::string newUrl = this->assembleRedirectionUrl(req);
			res.addHeader("Location", newUrl);
			return redirection.statusCode;
		}

		int statusCode = handlers[req.getHttpMethod()](*this, req, res);
		if (statusCode == 200)
			return 200;

		return this->returnErrPage(statusCode, res);
	}

	int listFiles(HttpResponse &res, std::string reqUrl)
	{
		std::string dirPath = root + reqUrl;
		DIR *dirStream = opendir(&dirPath[0]);
		if (!dirStream)
		{
			std::cerr << "opendir: " << strerror(errno) << "\n";
			return 500;
		}

		struct dirent *entry = readdir(dirStream);
		if (!entry)
		{
			std::cerr << "readdir: " << strerror(errno) << "\n";
			return 500;
		}

		std::string listingPage = createListingDirPage(reqUrl, entry, dirStream);
		if (listingPage == "")
			return 500;

		res.set(200, "OK", ".html", listingPage);
		return 200;
	}

	std::string createListingDirPage(std::string reqUrl, struct dirent *entry, DIR *dirStream)
	{
		std::string page;

		page = "<!DOCTYPE html><html><head><title>Index of " + reqUrl + "</title></head>";
		page += "<body style='font-family: monospace;'><h1>Index of " + reqUrl + "</h1><hr><pre style='display: flex;flex-direction: column;'>";

		errno = 0;
		while (entry)
		{
			std::string entry_name = entry->d_name;
			if (entry_name != ".")
			{
				int isDir = isDirectory(root + reqUrl + "/" + entry->d_name);
				if (isDir == -1)
					return "";
				if (isDir)
					entry_name += "/";

				page += "<a href='" + entry_name + "'>" + entry_name + "</a>";
			}

			entry = readdir(dirStream);
			if (!entry && errno != 0)
			{
				std::cerr << "readdir: " << strerror(errno) << "\n";
				return "";
			}
		}

		page += "</pre><hr></body></html>";
		return page;
	}

	std::string assembleRedirectionUrl(HttpRequest &req)
	{
		if (redirection.url.find("http:") == 0 || redirection.url.find("https:") == 0)
		{
			return redirection.url;
		}

		if (redirection.url.find_first_of("/") == 0)
		{
			return "http://" + req.getHostName() + ":" + req.getHostPort() + redirection.url;
		}

		return redirection.url;
	}

	void inheritServerBlock(Block &block)
	{
		if (redirection.url == "")
			redirection = block.redirection;
		if (indexFiles.size() == 0)
			indexFiles = block.indexFiles;
		if (root == "")
			root = block.root;
	}
};

#endif