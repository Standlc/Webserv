#include "PollFd.hpp"

ProxyPoll::ProxyPoll(int socket, ClientPoll &client, ProxyUrl &proxyPass) : PollFd(socket, client.server()),
                                                                            _client(client),
                                                                            _proxyReq(client.req(), proxyPass),
                                                                            _proxyRes(client.res()) {
    client.setProxyStatus(_status);
    _clientStatus = client.getStatus();
}

ProxyPoll::~ProxyPoll() {
}

ClientPoll &ProxyPoll::client() {
    return _client;
}

int ProxyPoll::clientStatus() {
    return *_clientStatus;
}

ProxyRequest &ProxyPoll::proxyReq() {
    return _proxyReq;
}

ProxyResponse &ProxyPoll::proxyRes() {
    return _proxyRes;
}

void ProxyPoll::setReadHandler(ProxyPollHandlerType f) {
    _readHandler = f;
}

void ProxyPoll::setWriteHandler(ProxyPollHandlerType f) {
    _writeHandler = f;
}

int ProxyPoll::handleRead(PollFd *pollFd) {
    if (_readHandler == NULL) {
        return 0;
    }
    return _readHandler((ProxyPoll *)pollFd);
}

int ProxyPoll::handleWrite(PollFd *pollFd) {
    if (_writeHandler == NULL) {
        return 0;
    }
    return _writeHandler((ProxyPoll *)pollFd);
}

int recvProxyResponse(ProxyPoll *proxy) {
    if (proxy->clientStatus() != 0) {
        return -1;
    }

    ProxyResponse &proxyRes = proxy->proxyRes();
    debug(">> reading proxy response", toString(proxy->getFd()), BLUE);
    int recvStatus = proxyRes.recvAll(proxy->getFd());
    if (recvStatus == -1) {
        return 500;
    }
    if (!recvStatus && !proxyRes.isComplete()) {
        return 502;
    }

    try {
        if (!proxyRes.isComplete() && proxyRes.resumeParsing()) {
            debugParsingSuccesss(proxyRes, proxy->getFd(), BLUE);
            proxyRes.setClientRes();
            proxy->client().setWriteHandler(sendResponseToClient);
            return -1;
        }
    } catch (int parsingErr) {
        debugParsingErr(proxyRes, proxy->getFd(), DIM_RED);
        return 502;
    }
    return 0;
}

int timeoutProxy(ProxyPoll *proxy) {
    if (proxy->clientStatus() != 0 || checkTimeout(proxy->startTime(), 15)) {
        return 504;
    }
    return 0;
}

int sendProxyRequest(ProxyPoll *proxy) {
    if (proxy->clientStatus() != 0) {
        return -1;
    }

    int sendStatus = proxy->proxyReq().send(proxy->getFd());
    if (sendStatus == 0) {
        proxy->setRemoveOnHungUp(false);
        proxy->setReadHandler(recvProxyResponse);
        proxy->setWriteHandler(timeoutProxy);
    }
    return (sendStatus == -1) ? 500 : 0;
}