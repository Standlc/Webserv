#ifndef SERVER_HPP
#define SERVER_HPP

#include "PollEvents/PollFd.hpp"
#include "blocks/Block.hpp"
#include "webserv.hpp"

struct CgiSockets;

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

    // size_t findPollFd(int fd);
    // CgiPoll& pushNewCgiPoll(int* cgiSockets, ClientPoll& client);
    // void pushNewCgiResponsePoll(int* cgiSockets, ClientPoll& client);
    // void pushNewCgiRequestPoll(int* cgiSockets, ClientPoll& client);

    void loadDefaultErrPage(int statusCode, HttpResponse& res);
    String getDefaultErrorPagePath(int statusCode);

    void removePollFd(int index);
    void pushNewClient(int clientSocket);
    void pushNewListeningSocket(int listeningSocket);
    CgiPoll& pushNewCgiPoll(CgiSockets& cgiSockets, ClientPoll& client);
    void pushStructPollfd(int fd);

    ServerBlock& findServerBlock(String port, String host);
    void addBlocks(int size);
    void addLocationBlocks(int serverIndex, int size);
    ServerBlock& getServerBlock(int index);
    LocationBlock& getLocationBlock(int serverIndex, int locationIndex);
};

#endif