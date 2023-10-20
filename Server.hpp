#include "webserv.hpp"

class Server
{
private:
	std::vector<struct pollfd> _sockets;
	std::vector<int> _listeningSockets;

	std::map<std::string, std::string> _statusComments;

public:
	std::vector<ServerBlock> blocks;

	Server()
	{
		_statusComments["200"] = "OK";
		_statusComments["400"] = "Bad Request";
		_statusComments["403"] = "Forbidden Resource";
		_statusComments["404"] = "Resource Not Found";
		_statusComments["405"] = "Method Not Allowed";
		_statusComments["500"] = "Internal Server Error";
		_statusComments["501"] = "Not Implemented";
	}

	int listen()
	{
		std::vector<std::string> usedPorts;

		for (int i = 0; i < blocks.size(); i++)
		{
			if (std::find(usedPorts.begin(), usedPorts.end(), blocks[i].port) != usedPorts.end())
				continue;

			struct addrinfo *serverInfo = getOwnAddressInfo(&blocks[i].port[0]);
			if (serverInfo == NULL)
				return -1;

			int socket = createBindedNonBlockingSocket(serverInfo);
			if (socket == -1 || bindSocket(socket, serverInfo) == -1)
			{
				freeaddrinfo(serverInfo);
				return -1;
			}
			freeaddrinfo(serverInfo);

			if (listenToSocket(socket, blocks[i].port) == -1)
				return -1;

			_listeningSockets.push_back(socket);
			this->pushSocket(socket);
			usedPorts.push_back(blocks[i].port);

			// std::cout << "Listening Socket: " << socket << "\n";
		}
		return 0;
	}

	int monitorClients()
	{
		while (true)
		{
			int readableSocketsCount = poll(&_sockets[0], _sockets.size(), -1);
			if (readableSocketsCount == -1)
			{
				std::cerr << "poll: " << strerror(errno) << "\n";
				return -1;
			}
			// std::cout << "SCANING READABLE _SOCKETS\n";
			this->scanForReadableSockets(readableSocketsCount);
		}
		return 0;
	}

	int scanForReadableSockets(int readableSocketsCount)
	{
		int readableSocketsFound = 0;

		for (int i = 0; i < _sockets.size() && readableSocketsFound < readableSocketsCount; i++)
		{
			// std::cout << "ITERATING THRU CLIENTS\n";
			bool isReadable = (_sockets[i].revents & POLLIN) == 1;
			if (!isReadable)
				continue;

			bool isListeningSocket = std::find(_listeningSockets.begin(), _listeningSockets.end(), _sockets[i].fd) != _listeningSockets.end();
			if (isListeningSocket)
				this->handleNewConnection(_sockets[i].fd);
			else
			{
				// std::cout << "Reading: " << _sockets[i].fd << "\n";
				this->handleClientRequest(_sockets[i].fd);
				close(_sockets[i].fd);
				_sockets.erase(_sockets.begin() + i);
				i--;
			}

			readableSocketsFound++;
		}
		return 0;
	}

	int handleNewConnection(int socket)
	{
		int newClientSocket;

		newClientSocket = accept(socket, NULL, NULL);
		if (newClientSocket == -1)
		{
			std::cerr << "accept: " << strerror(errno) << "\n";
			return -1;
		}

		this->pushSocket(newClientSocket);
		return 0;
	}

	int handleClientRequest(int socket)
	{
		HttpRequest req;
		HttpResponse res;

		std::string statusCode = req.parseRequest(socket);

		auto block = this->findServerBlock(getSocketPort(socket), req.getHostName());
		if (statusCode == "200")
			statusCode = block->execute(req, res);

		if (statusCode != "200")
			handleStatusCode(statusCode, *block, res);

		// std::cout << req.getRawData() << "\n";
		// std::cout << res.getResponse() << "\n";

		if (res.sendAll(socket) == -1)
			std::cerr << "Error while sending response\n";
		return 0;
	}

	int handleStatusCode(std::string statusCode, ServerBlock &block, HttpResponse &res)
	{
		std::string errorPagePath = block.root + "/" + statusCode + ".html";

		std::string loadFileStatus = res.loadFile(statusCode, errorPagePath, _statusComments[statusCode]);
		if (loadFileStatus == "200")
			return 0;

		if (loadFileStatus != "500" && !(this->loadDefaultErrorPage(statusCode, res)))
			return 0;

		std::string body = "The server encoutered some issue while handling your request";
		res.set("500", _statusComments["500"], ".txt", body);
		return 1;
	}

	bool loadDefaultErrorPage(std::string statusCode, HttpResponse &res)
	{
		std::string defaultErrorPagePath = "defaultPages/" + statusCode + ".html";
		if (res.loadFile(statusCode, defaultErrorPagePath, _statusComments[statusCode]) == "200")
			return 0;
		return 1;
	}

	std::vector<ServerBlock>::iterator findServerBlock(std::string port, std::string hostName)
	{
		std::vector<ServerBlock>::iterator defaultBlock = blocks.begin();

		for (auto block = blocks.begin(); block < blocks.end(); block++)
		{
			if (block->port == port)
			{
				// if (block->hostNames.size())
				// 	std::cout << block->hostNames[0] << "\n";
				// std::cout << hostName << "\n";
				if (std::find(block->hostNames.begin(), block->hostNames.end(), hostName) != block->hostNames.end())
					return block;
				if (block->isDefault)
					defaultBlock = block;
			}
		}
		return defaultBlock;
	}

	void pushSocket(int socketFd)
	{
		struct pollfd pollfd_socket;

		pollfd_socket.fd = socketFd;
		pollfd_socket.events = POLLIN | POLLOUT;
		pollfd_socket.revents = 0;
		_sockets.push_back(pollfd_socket);
	}
};