#include "../Block.hpp"

String parseLinkHostName(const String &link) {
    size_t start = link.find("://") + 3;
    size_t end = link.find_first_of(':', start);
    if (start == NPOS) {
        return "";
    }
    return link.substr(start, end - start);
}

String getLinkConnectionPort(const String &link) {
    size_t hostNamePos = link.find("://") + 3;
    size_t portPos = link.find_first_of(':', hostNamePos);
    if (portPos == NPOS) {
        if (startsWith(link, "http://")) {
            return "80";
        }
        if (startsWith(link, "https://")) {
            return "443";
        }
        return "";
    }

    portPos += 1;
    size_t portEndPos = link.find_first_of('/', portPos);
    String port = link.substr(portPos, portEndPos - portPos);
    return port;
}

void connectToSocket(int socket, struct addrinfo *addrInfo) {
    if (connect(socket, addrInfo->ai_addr, addrInfo->ai_addrlen) == -1) {
        debugErr("connect", strerror(errno));
        freeaddrinfo(addrInfo);
        closeOpenFd(socket);
        throw 1;
    }
}

clientPollHandlerType LocationBlock::proxyHandler(ClientPoll &client) {
    struct addrinfo *proxyInfo = NULL;
    String hostName = parseLinkHostName(_proxyPass);
    String port = getLinkConnectionPort(_proxyPass);
    int proxySocket = 0;

    try {
        proxyInfo = getServerAddressInfo(hostName, port);
        proxySocket = createSocket(proxyInfo);
        debug("> connecting to remote server", std::to_string(proxySocket), BLUE);
        connectToSocket(proxySocket, proxyInfo);
        freeaddrinfo(proxyInfo);
    } catch (int err) {
        throw 502;
    }

    client.server().pushNewProxy(proxySocket, client, hostName + ":" + port);
    return checkProxyPoll;
}
