#ifndef SERVER_BLOCK
#define SERVER_BLOCK

#include "webserv.hpp"
#include "LocationBlock.hpp"

int compare(std::string str1, std::string str2)
{
	int i = 0;
	for (; i <= str1[i] != '\0' && str2[i] != '\0'; i++)
	{
		if (str1[i] != str2[i])
			return i;
	}
	return i;
}

typedef std::vector<LocationBlock> locationBlocks;

class ServerBlock : public Block
{
private:
public:
	bool isDefault;
	std::string port;
	std::vector<std::string> hostNames;

	int execute(HttpRequest &req, HttpResponse &res)
	{
		int statusCode = 200;
		LocationBlock *macthingLocation = this->findLocationBlockByPath(req.getUrl());

		if (macthingLocation == NULL)
			statusCode = 404;
		else if (!macthingLocation->handlers.count(req.getHttpMethod()))
			statusCode = 501;

		if (statusCode == 200)
			statusCode = macthingLocation->execute(req, res);

		if (statusCode != 200)
			return this->returnErrPage(statusCode, res);
		return 200;
	}

	LocationBlock *findLocationBlockByPath(std::string reqUrl)
	{
		int maxMatchLen = 0;
		int matchIndex = -1;

		std::cout << reqUrl << "\n";

		for (int i = 0; i < _locationBlocks.size(); i++)
		{
			if (_locationBlocks[i].path == reqUrl)
				return &_locationBlocks[i];
			if (_locationBlocks[i].isExact)
				continue;

			int compLen = compare(_locationBlocks[i].path, reqUrl);
			if (compLen <= maxMatchLen || compLen < _locationBlocks[i].path.length() - 1)
				continue;

			char blockChar = _locationBlocks[i].path[compLen];
			char urlChar = reqUrl[compLen];
			if ((blockChar == '/' && urlChar == '\0') || (blockChar == '\0' && (urlChar == '/' || reqUrl[compLen - 1] == '/')))
			{
				maxMatchLen = compLen;
				matchIndex = i;
			}
		}
		return matchIndex == -1 ? NULL : &_locationBlocks[matchIndex];
	}

	std::vector<LocationBlock> _locationBlocks;
};

#endif