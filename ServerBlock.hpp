#ifndef SERVER_BLOCK
#define SERVER_BLOCK

#include "webserv.hpp"
#include "Server.hpp"
#include "Block.hpp"
#include "LocationBlock.hpp"
#include "PollFd.hpp"

bool isUnkownMethod(std::string method)
{
	return method != "GET" && method != "POST" && method != "DELETE";
}

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

class ClientPollFd;

class ServerBlock : public Block
{
private:
public:
	bool isDefault;
	std::string port;
	std::vector<std::string> hostNames;

	void execute(Server &server, ClientPollFd &client);

	void handleCgi(Server &server, ClientPollFd &client);

	LocationBlock *findLocationBlockByPath(std::string reqUrl)
	{
		int locationBlocksSize = _locationBlocks.size();
		size_t maxMatchLen = 0;
		int matchIndex = -1;

		for (int i = 0; i < locationBlocksSize; i++)
		{
			if (_locationBlocks[i].path == reqUrl)
				return &_locationBlocks[i];
			if (_locationBlocks[i].isExact)
				continue;

			size_t compLen = compare(_locationBlocks[i].path, reqUrl);
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