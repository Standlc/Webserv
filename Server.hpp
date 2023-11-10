#ifndef SERVER_HPP
#define SERVER_HPP

class Server;
class PollFd;

#include "webserv.hpp"
#include "ServerBlock.hpp"
#include "LocationBlock.hpp"
#include "PollFd.hpp"

bool isReadable(struct pollfd &pollEl)
{
	return (pollEl.revents & POLLIN) == 1;
}

bool isWritable(struct pollfd &pollEl)
{
	return (pollEl.revents & POLLOUT);
}

int checkPollError(struct pollfd &pollEl, int error)
{
	if (pollEl.revents & error)
	{
		std::cout << "POLL ERROR " << error << "\n";
		return error;
	}
	return 0;
}

int checkPollErrors(struct pollfd &pollEl)
{
	return (
		checkPollError(pollEl, POLLERR) ||
		checkPollError(pollEl, POLLHUP) ||
		checkPollError(pollEl, POLLNVAL));
}

int sendResponseToClient(Server &server, PollFd *pollFd);
int handleClientRequest(Server &server, PollFd *pollFd);
int handleNewConnection(Server &server, PollFd *pollFd);
int readClientRequest(Server &server, PollFd *pollFd);
int waitCgiResponse(Server &server, PollFd *pollFd);
int sendCgiResponseToClient(Server &server, PollFd *pollFd);
int readCgiResponseFromPipe(Server &server, PollFd *pollFd);

class Server
{
private:
	std::vector<struct pollfd> _fds;
	int _fdsSize;
	std::vector<PollFd *> _pollFds;

public:
	std::vector<ServerBlock> blocks;

	Server() {}

	int listen()
	{
		int blocksSize = blocks.size();
		std::vector<std::string> usedPorts;

		for (int i = 0; i < blocksSize; i++)
		{
			if (std::find(usedPorts.begin(), usedPorts.end(), blocks[i].port) != usedPorts.end())
				continue;

			struct addrinfo *serverInfo = getOwnAddressInfo(&blocks[i].port[0]);
			if (serverInfo == NULL)
				return -1;

			int socket = createBindedSocket(serverInfo);
			if (socket == -1 || bindSocket(socket, serverInfo) == -1)
			{
				freeaddrinfo(serverInfo);
				return -1;
			}
			freeaddrinfo(serverInfo);

			if (listenToSocket(socket, blocks[i].port) == -1)
				return -1;

			PollFd *listeningPollFd = new PollFd(socket);
			this->pushPollFd(listeningPollFd, handleNewConnection, NULL);
			usedPorts.push_back(blocks[i].port);
		}
		return 0;
	}

	int monitorClients()
	{
		while (true)
		{
			int eventSocketCount = poll(&_fds[0], _fds.size(), -1);
			if (eventSocketCount == -1)
			{
				std::cerr << "poll: " << strerror(errno) << "\n";
				return -1;
			}
			this->scanForEventSockets(eventSocketCount);
		}
		return 0;
	}

	void scanForEventSockets(int eventSocketCount)
	{
		int eventSocketFound = 0;
		int returnStatus = 0;
		_fdsSize = _fds.size();

		// std::cout << "\nsize: " << _fdsSize << '\n';
		for (int _i = 0; _i < _fdsSize && eventSocketFound < eventSocketCount; _i++)
		{
			if (isReadable(_fds[_i]))
			{
				// std::cout << "READABLE\n";
				returnStatus = _pollFds[_i]->handleRead(*this, _pollFds[_i]);
				eventSocketFound++;
			}
			else if (isWritable(_fds[_i]))
			{
				// std::cout << "WRITABLE\n";
				returnStatus = _pollFds[_i]->handleWrite(*this, _pollFds[_i]);
				eventSocketFound++;
			}

			if (returnStatus != 0 || checkPollErrors(_fds[_i]))
			{
				// std::cout << "REMOVING FD: " << _fds[_i].fd <<  "\n\n";
				this->removePollFd(_i);
				_i -= 1;
			}
		}
	}

	void removePollFd(int index)
	{
		int fd = _fds[index].fd;
		close(fd);

		delete _pollFds[index];
		_pollFds.erase(_pollFds.begin() + index);

		_fds.erase(_fds.begin() + index);
		_fdsSize--;
	}

	void pushPollFd(PollFd *pollFd, pollFdHandlerType readHandler, pollFdHandlerType writeHandler)
	{
		// fcntl(fd, F_SETFL, O_NONBLOCK);
		struct pollfd pollfd_socket;
		pollfd_socket.fd = pollFd->getFd();
		pollfd_socket.events = POLLIN | POLLOUT;
		_fds.push_back(pollfd_socket);

		pollFd->setReadHandler(readHandler);
		pollFd->setWriteHandler(writeHandler);
		_pollFds.push_back(pollFd);
	}

	void loadDefaultErrPage(int statusCode, HttpResponse &res)
	{
		try
		{
			std::string errPagePath = "defaultPages/error/" + std::to_string(statusCode) + ".html";
			res.loadFile(statusCode, errPagePath);
		}
		catch (int status)
		{
			std::string body = "The server encountered some issue while handling your request";
			res.set(status, ".txt", &body);
		}
	}

	ServerBlock *findServerBlock(std::string port, std::string host)
	{
		int blocksSize = blocks.size();
		std::string hostName = host.substr(0, host.find_first_of(':'));
		ServerBlock *defaultBlock = &blocks[0];

		for (int i = 0; i < blocksSize; i++)
		{
			if (blocks[i].port == port)
			{
				if (std::find(blocks[i].hostNames.begin(), blocks[i].hostNames.end(), hostName) != blocks[i].hostNames.end())
					return &blocks[i];
				if (blocks[i].isDefault)
					defaultBlock = &blocks[i];
			}
		}
		return defaultBlock;
	}

	int getPollFdIndex(int fd)
	{
		int size = _pollFds.size();
		for (int i = 0; i < size; i++)
		{
			if (_pollFds[i]->getFd() == fd)
				return i;
		}
		return -1;
	}
};

int sendResponseToClient(Server &server, PollFd *pollFd)
{
	ClientPollFd &client = *(ClientPollFd *)(pollFd);
	HttpResponse &res = client.getRes();
	int clientSocket = client.getFd();

	std::cout << "res isSet(), sending response back to client\n";

	if (res.sendAll(clientSocket) <= 0)
	{
		return -1;
	}
	return 0;
}

int handleNewConnection(Server &server, PollFd *pollFd)
{
	std::cout << "\nnew client\n";

	int newClientSocket = accept(pollFd->getFd(), NULL, NULL);
	if (newClientSocket == -1)
	{
		std::cerr << "\033[31mR\033[0maccept: " << strerror(errno) << "\n";
		return 0;
	}

	ClientPollFd *newClient = new ClientPollFd(newClientSocket);
	server.pushPollFd(newClient, readClientRequest, NULL);
	return 0;
}

int readClientRequest(Server &server, PollFd *pollFd)
{
	ClientPollFd &client = *(ClientPollFd *)(pollFd);
	int clientSocket = client.getFd();
	HttpRequest &req = client.getReq();

	std::cout << clientSocket << " reading client request\n";

	if (req.recvAll(clientSocket) == -1)
	{
		std::cout << "\033[31mR\033[0mencountered error while reading client socket\n";
		return 0;
	}

	client.setWriteHandler(handleClientRequest);
	return 0;
}

int handleClientRequest(Server &server, PollFd *pollFd)
{
	ClientPollFd &client = *(ClientPollFd *)(pollFd);
	int clientSocket = client.getFd();
	HttpRequest &req = client.getReq();
	HttpResponse &res = client.getRes();

	try
	{
		std::cout << "parsing and executing request\n";
		req.parseRequest();
		std::string socketPort = getSocketPort(clientSocket);
		ServerBlock *block = server.findServerBlock(socketPort, req.getHeader("Host"));
		block->execute(server, client);
	}
	catch (int statusCode)
	{
		server.loadDefaultErrPage(statusCode, res);
	}

	if (res.isSet() == false)
		return 0;

	client.setWriteHandler(sendResponseToClient);
	return sendResponseToClient(server, pollFd);
}

int readCgiResponseFromPipe(Server &server, PollFd *pollFd)
{
	CgiPollFd &cgiPollFd = *(CgiPollFd *)(pollFd);
	HttpResponse &res = cgiPollFd.getRes();
	int pipeOutput = cgiPollFd.getFd();
	ClientPollFd &client = cgiPollFd.getClient();

	std::cout << "reading cgi response from pipe: " << cgiPollFd.getFd() << "\n";

	client.setWriteHandler(waitCgiResponse);
	if (res.recvAll(pipeOutput) == -1)
	{
		std::cout << "\033[31mR\033[0mcgi encountered an error while reading pipe\n";
		server.loadDefaultErrPage(500, res);
		kill(cgiPollFd.getPid(), SIGKILL);
		client.setWriteHandler(sendCgiResponseToClient);
		return -1;
	}
	return 0;
}

int sendCgiResponseToClient(Server &server, PollFd *pollFd)
{
	ClientPollFd &client = *(ClientPollFd *)(pollFd);
	HttpResponse &res = client.getRes();
	int clientSocket = client.getFd();

	std::cout << "sending cgi response\n";

	if (res.sendAll(clientSocket) <= 0)
	{
		std::cout << "done sending cgi res\n";
		std::cout << res.getOuputData();
		return -1;
	}
	return 0;
}

int removeCgiPollFd(Server &server, PollFd *pollFd)
{
	ClientPollFd &client = *(ClientPollFd *)(pollFd);
	int *cgiPipes = client.getCgiPipes();
	int cgiPollFdIndex = server.getPollFdIndex(cgiPipes[0]);

	if (cgiPollFdIndex != -1)
	{
		std::cout << "removing cgi pipe: " << cgiPipes[0] << " from poll()\n";
		close(cgiPipes[1]);
		server.removePollFd(cgiPollFdIndex);
	}

	client.setWriteHandler(sendCgiResponseToClient);
	return sendCgiResponseToClient(server, pollFd);
}

int waitCgiResponse(Server &server, PollFd *pollFd)
{
	ClientPollFd &client = *(ClientPollFd *)(pollFd);
	HttpResponse &res = client.getRes();
	int cgiPid = client.getCgiPid();

	if (waitpid(cgiPid, NULL, WNOHANG) == -1)
	{
		std::cout << "cgi has exit()\n";
		res.set(200);
		client.setWriteHandler(removeCgiPollFd);
	}
	return 0;
}

////////////////////////////////////////////////////////////

void ServerBlock::execute(Server &server, ClientPollFd &client)
{
	HttpResponse &res = client.getRes();
	HttpRequest &req = client.getReq();

	LocationBlock *macthingLocation = this->findLocationBlockByPath(req.getUrl());
	try
	{
		if (isUnkownMethod(req.getHttpMethod()))
			throw 501;
		if (req.getUrl()[0] != '/' || req.getUrl().find("../") != (size_t)-1)
			throw 400;
		if (req.getProtocol() != "HTTP/1.1")
			throw 400;
		if (macthingLocation == NULL)
			throw 404;
		if (!macthingLocation->handlers.count(req.getHttpMethod()))
			throw 405;

		if (macthingLocation->cgiExtensions.size() > 0)
		{
			if (macthingLocation->cgiExtensions.count(getFileExtension(req.getUrl())))
			{
				this->handleCgi(server, client);
				return;
			}
		}

		macthingLocation->execute(req, res);
	}
	catch (int status)
	{
		this->loadErrPage(status, res);
	}
}

void ServerBlock::handleCgi(Server &server, ClientPollFd &client)
{
	// THROW ERRORS!!!

	int fds[2];
	pipe(fds);
	write(fds[1], "\r\nhello", 8);
	int pid = 1234;

	client.setCgiPipes(fds);
	client.setCgiPid(pid);
	client.setWriteHandler(waitCgiResponse);

	CgiPollFd *cgiPollFd = new CgiPollFd(pid, fds, client.getFd(), client.getRes(), client);
	server.pushPollFd(cgiPollFd, readCgiResponseFromPipe, NULL);
}

#endif