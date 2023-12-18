#include "../Block.hpp"

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
    int proxySocket = 0;

    try {
        proxyInfo = getServerAddressInfo(_proxyPass->host(), _proxyPass->port());
        proxySocket = createSocket(proxyInfo);
        debug("> connecting to remote server", toString(proxySocket), BLUE);
        connectToSocket(proxySocket, proxyInfo);
        freeaddrinfo(proxyInfo);
    } catch (int err) {
        throw 502;
    }

    client.server().pushNewProxy(proxySocket, client, *_proxyPass);
    return checkProxyPoll;
}
