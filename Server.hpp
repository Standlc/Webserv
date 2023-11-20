#ifndef SERVER_HPP
#define SERVER_HPP

#include "PollEvents/PollFd.hpp"
#include "blocks/Block.hpp"
#include "webserv.hpp"

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

    int listen();
    int monitorClients();
    void scanForEventSockets(int eventsAmount);

    void removePollFd(int index);
    void pushNewClient(int clientSocket);
    void pushNewListeningSocket(int listeningSocket);
    void pushNewCgiResponsePoll(int* pipes, ClientPoll& client);
    void pushNewCgiRequestPoll(int* pipes, ClientPoll& client);
    void pushStructPollfd(int fd);

    void loadDefaultErrPage(int statusCode, HttpResponse& res);

    String getDefaultErrorPagePath(int statusCode);
    ServerBlock& findServerBlock(String port, String host);

    size_t findPollFd(int fd);

    void addBlocks(int size);
    void addLocationBlocks(int serverIndex, int size);
    ServerBlock& getServerBlock(int index);
    LocationBlock& getLocationBlock(int serverIndex, int locationIndex);
};

#endif