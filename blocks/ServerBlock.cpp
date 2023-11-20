#include "Block.hpp"

ServerBlock::ServerBlock() : Block(), _isDefault(false), _locationBlockSize(0){};

ServerBlock &ServerBlock::operator=(const ServerBlock &b) {
    Block::operator=(b);
    _port = b._port;
    _isDefault = b._isDefault;
    _hostNames = b._hostNames;
    _locationBlockSize = b._locationBlockSize;
    return *this;
}

clientPollHandlerType ServerBlock::execute(Server &server, ClientPoll &client) {
    LocationBlock *macthingLocation = this->findLocationBlockByPath(client.req().url().path);

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

LocationBlock *ServerBlock::findLocationBlockByPath(const String &reqPath) {
    int maxMatchLen = 0;
    int matchIndex = -1;
    int reqSize = reqPath.size();

    for (int i = 0; i < _locationBlockSize; i++) {
        if (_locationBlocks[i].getPath() == reqPath) {
            return &_locationBlocks[i];
        }
        if (_locationBlocks[i].isExactPath()) {
            continue;
        }

        String locationPath = _locationBlocks[i].getPath();

        int pathSize = locationPath.size();
        if (pathSize > maxMatchLen && reqPath.compare(0, pathSize, locationPath) == 0) {
            if (reqPath[pathSize] == '/' || locationPath.back() == '/') {
                maxMatchLen = pathSize;
                matchIndex = i;
            }
        }

        if (reqSize > maxMatchLen && locationPath.compare(0, reqSize, reqPath) == 0) {
            if (locationPath.find_first_not_of('/', reqSize) == NPOS) {
                maxMatchLen = reqSize;
                matchIndex = i;
            }
        }
    }
    return (matchIndex == -1) ? NULL : &_locationBlocks[matchIndex];
}

void ServerBlock::addLocationBlocks(int size) {
    _locationBlockSize += size;
    LocationBlock location(*this);
    for (int i = 0; i < size; i++) {
        _locationBlocks.push_back(location);
    }
}

LocationBlock &ServerBlock::getLocationBlock(int index) {
    return _locationBlocks[index];
}

void ServerBlock::set(String ipAddress, String port, bool isDefault) {
    _ipAddress = ipAddress;
    _port = port;
    _isDefault = isDefault;
}

void ServerBlock::setHostName(String name) {
    _hostNames.push_back(name);
}

bool ServerBlock::isHost(String hostName) {
    return std::find(_hostNames.begin(), _hostNames.end(), hostName) != _hostNames.end();
}

String ServerBlock::getPort() {
    return _port;
}

String ServerBlock::getIpAddress() {
    return _ipAddress;
}

bool ServerBlock::isDefault() {
    return _isDefault;
}