#ifndef SERVER_BLOCK
#define SERVER_BLOCK

#include "webserv.hpp"
#include "LocationBlock.hpp"

typedef std::vector<LocationBlock> locationBlocks;

class ServerBlock : public Block
{
private:
public:
	bool isDefault;
	std::string port;
	std::vector<std::string> hostNames;

	std::string execute(HttpRequest &req, HttpResponse &res)
	{
		std::string statusCode = "200";

		LocationBlock *macthingLocation = this->findLocationBlockByPath(req.getUrl());

		if (macthingLocation == NULL)
			statusCode = "404";
		else if (!macthingLocation->handlers.count(req.getHttpMethod()))
			statusCode = "501";

		if (statusCode == "200")
			statusCode = macthingLocation->execute(req, res);

		// std::cout << statusCode << " executing location block\n";

		if (statusCode != "200")
			return this->returnErrPage(statusCode, res);
		return "200";
	}

	LocationBlock *findLocationBlockByPath(std::string reqUrl)
	{
		int urlLen = reqUrl.length();
		int maxMatchLen = 0;
		int matchingIndex = -1;

		for (int i = 0; i < _locationBlocks.size(); i++)
		{
			if (_locationBlocks[i].isExact)
			{
				if (_locationBlocks[i].path == reqUrl)
					return &_locationBlocks[i];
				continue;
			}

			int blockpathLen = _locationBlocks[i].path.length();
			if (blockpathLen > urlLen || blockpathLen <= maxMatchLen)
				continue;

			if (reqUrl.find(_locationBlocks[i].path) != std::string::npos)
			{
				maxMatchLen = blockpathLen;
				matchingIndex = i;
			}
		}
		return matchingIndex == -1 ? NULL : &_locationBlocks[matchingIndex];
	}

	std::vector<LocationBlock> _locationBlocks;
};

#endif