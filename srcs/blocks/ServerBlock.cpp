#include "Block.hpp"

ServerBlock::ServerBlock() : Block() {
    _ipAddress = "";
    _port = "";
    _isDefault = 0;
};

ServerBlock &ServerBlock::operator=(const ServerBlock &b) {
    Block::operator=(b);
    _port = b._port;
    _locations = b._locations;
    _isDefault = b._isDefault;
    _hostNames = b._hostNames;
    _ipAddress = b._ipAddress;
    return *this;
}

clientPollHandlerType ServerBlock::execute(ClientPoll &client) {
    try {
        LocationBlock *macthingLocation = this->findLocationBlockByPath(client.req().url().path);
        throwIf(macthingLocation == NULL, 404);
        client.setLocation(*macthingLocation);

        return macthingLocation->execute(client);
    } catch (int status) {
        this->loadErrPage(status, client.res(), "/");
        return sendResponseToClient;
    }
}

LocationBlock *ServerBlock::findLocationBlockByPath(const String &reqPath) {
    int maxMatchLen = 0;
    int matchIndex = -1;
    int reqSize = reqPath.size();

    for (size_t i = 0; i < _locations.size(); i++) {
        if (_locations[i].getPath() == reqPath) {
            return &_locations[i];
        }
        if (_locations[i].isExactPath()) {
            continue;
        }

        String locationPath = _locations[i].getPath();
        int pathSize = locationPath.size();
        if (pathSize > maxMatchLen && reqPath.compare(0, pathSize, locationPath) == 0) {
            if (reqPath[pathSize] == '/' || lastChar(locationPath) == '/') {
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
    return (matchIndex == -1) ? NULL : &_locations[matchIndex];
}

LocationBlock *ServerBlock::addLocation() {
    LocationBlock newLocation(*this);
    _locations.push_back(newLocation);
    return &_locations[_locations.size() - 1];
}

LocationBlock *ServerBlock::addLocation(LocationBlock &location) {
    LocationBlock newLocation(location);
    _locations.push_back(newLocation);
    return &_locations[_locations.size() - 1];
}

LocationBlock &ServerBlock::getLocationBlock(int index) {
    return _locations[index];
}

void ServerBlock::set(String ipAddress, String port, bool isDefault) {
    if (ipAddress == "") {
        _ipAddress = "0.0.0.0";
    } else {
        _ipAddress = ipAddress;
    }
    _port = port;
    _isDefault = isDefault;
}

void ServerBlock::addHostName(String name) {
    _hostNames.push_back(name);
}

bool ServerBlock::isHost(const String &hostName) {
    if (std::find(_hostNames.begin(), _hostNames.end(), hostName) != _hostNames.end()) {
        return true;
    }
    return _ipAddress == hostName;
}

const std::vector<std::string> &ServerBlock::hostNames() {
    return _hostNames;
}

const String &ServerBlock::port() {
    return _port;
}

const String &ServerBlock::ipAddress() {
    return _ipAddress;
}

bool ServerBlock::isDefault() {
    return _isDefault;
}

String ServerBlock::getIpAddress() {
    return (_ipAddress);
}

String ServerBlock::getPort() {
    return (_port);
}

std::vector<String> ServerBlock::getHostNames() {
    return (_hostNames);
}