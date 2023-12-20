#ifndef SERVER_HPP
#define SERVER_HPP

#include "PollEvents/PollFd.hpp"
#include "blocks/Block.hpp"
#include "blocks/LocationBlock/utils.hpp"
#include "webserv.hpp"

struct CgiSockets;
int createSocket(struct addrinfo* addrInfo);

class Server {
   private:
    std::vector<struct pollfd> _fds;
    std::vector<PollFd*> _pollFds;
    std::vector<ServerBlock> _blocks;
    int _serverBlockSize;
    int _fdsSize;

   public:
    Server();
    ~Server();
    void deleteResource();

    int listen();
    int startServers();
    int monitorClients();
    void scanForEventSockets(int eventsAmount);
    int handleSocketErrors(int* i, int socketStatus);
    int checkReadableANDWritable(int i);
    int checkReadableORWritable(int i);

    void loadDefaultErrPage(int statusCode, HttpResponse& res);
    String getDefaultErrorPagePath(int statusCode);

    void removePollFd(int index, int socketStatus);
    void pushNewClient(int clientSocket);
    void pushNewProxy(int proxySocket, ClientPoll& client, ProxyUrl& proxyPass);
    void pushNewListeningSocket(int listeningSocket);
    CgiPoll& pushNewCgiPoll(CgiSockets& cgiSockets, ClientPoll& client);
    void pushStructPollfd(int fd);

    ServerBlock* findServerBlock(HttpRequest& req);
    ServerBlock* addBlock();
    ServerBlock& getServerBlock(int index);
    int getServerSize();
};

#endif