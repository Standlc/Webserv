#ifndef SERVER_BLOCK
#define SERVER_BLOCK

#include "Block.hpp"
#include "LocationBlock.hpp"
#include "PollFd.hpp"
#include "Server.hpp"
#include "webserv.hpp"

int compare(std::string str1, std::string str2) {
    int i = 0;
    for (; i <= str1[i] != '\0' && str2[i] != '\0'; i++) {
        if (str1[i] != str2[i]) {
            return i;
        }
    }
    return i;
}

class ClientPollFd;
class LocationBlock;

class ServerBlock : public Block {
   private:
    std::string _port;
    bool _isDefault;
    std::vector<std::string> _hostNames;

    int _locationBlockSize;

   public:
    ServerBlock() : Block(), _isDefault(false), _locationBlockSize(0){};

    ServerBlock &operator=(const ServerBlock &b) {
        Block::operator=(b);
        _port = b._port;
        _isDefault = b._isDefault;
        _hostNames = b._hostNames;
        _locationBlockSize = b._locationBlockSize;
        return *this;
    }

    clientPollFdHandlerType execute(Server &server, ClientPollFd &client) {
        LocationBlock *macthingLocation = this->findLocationBlockByPath(client.req().getUrl());

        // std::cout << macthingLocation->getPath() << '\n';
        try {
            if (macthingLocation == NULL) {
                throw 404;
            }
            return macthingLocation->execute(server, client);
        } catch (int status) {
            this->loadErrPage(status, client.res(), client.req());
            return sendResponseToClient;
        }
    }

    LocationBlock *findLocationBlockByPath(std::string reqUrl) {
        int maxMatchLen = 0;
        int matchIndex = -1;
        int reqSize = reqUrl.size();

        for (int i = 0; i < _locationBlockSize; i++) {
            if (_locationBlocks[i].getPath() == reqUrl) {
                return &_locationBlocks[i];
            }
            if (_locationBlocks[i].isExactPath()) {
                continue;
            }

            std::string locationPath = _locationBlocks[i].getPath();

            int pathSize = locationPath.size();
            if (pathSize > maxMatchLen && reqUrl.compare(0, pathSize, locationPath) == 0) {
                if (reqUrl[pathSize] == '/' || locationPath.back() == '/') {
                    maxMatchLen = pathSize;
                    matchIndex = i;
                }
            }

            if (reqSize > maxMatchLen && locationPath.compare(0, reqSize, reqUrl) == 0) {
                if (locationPath.find_first_not_of('/', reqSize) == (size_t)-1) {
                    maxMatchLen = reqSize;
                    matchIndex = i;
                }
            }
        }
        return (matchIndex == -1) ? NULL : &_locationBlocks[matchIndex];
    }

    void addLocationBlocks(int size) {
        _locationBlockSize += size;
        LocationBlock location(*this);
        for (int i = 0; i < size; i++) {
            _locationBlocks.push_back(location);
        }
    }

    LocationBlock &getLocationBlock(int index) {
        return _locationBlocks[index];
    }

    void set(std::string port, bool isDefault) {
        _port = port;
        _isDefault = isDefault;
    }

    void setHostName(std::string name) {
        _hostNames.push_back(name);
    }

    bool isHost(std::string hostName) {
        return std::find(_hostNames.begin(), _hostNames.end(), hostName) != _hostNames.end();
    }

    std::string getPort() {
        return _port;
    }

    bool isDefault() {
        return _isDefault;
    }

    std::vector<LocationBlock> _locationBlocks;
};

#endif