#ifndef POLL_FD_HPP
#define POLL_FD_HPP

#include "webserv.hpp"
#include "Server.hpp"
#include "LocationBlock.hpp"

class PollFd;

typedef int (*pollFdHandlerType)(Server &, PollFd *pollFd);

class PollFd
{
protected:
	// LocationBlock *_locationBlock;
	pollFdHandlerType _readHandler;
	pollFdHandlerType _writeHandler;
	int _fd;

public:
	PollFd(int fd) : _fd(fd)
	{
		_readHandler = NULL;
		_writeHandler = NULL;
		// _locationBlock = NULL;
	}

	// void setLocationBlock(LocationBlock *locationBlock)
	// {
	// 	_locationBlock = locationBlock;
	// }

	// LocationBlock *getLocationBlock()
	// {
	// 	return _locationBlock;
	// }

	int getFd()
	{
		return _fd;
	}

	void setWriteHandler(pollFdHandlerType f)
	{
		_writeHandler = f;
	}

	void setReadHandler(pollFdHandlerType f)
	{
		_readHandler = f;
	}

	int handleWrite(Server &server, PollFd *pollFd)
	{
		if (_writeHandler == NULL)
			return 0;
		return _writeHandler(server, pollFd);
	}

	int handleRead(Server &server, PollFd *pollFd)
	{
		if (_readHandler == NULL)
			return 0;
		return _readHandler(server, pollFd);
	}
};

class ClientPollFd : public PollFd
{
private:
	int (*_)(Server &, PollFd *pollFd);
	HttpRequest _req;
	HttpResponse _res;
	int _cgiPid;
	int _cgiPipes[2];

public:
	ClientPollFd(int &fd) : PollFd(fd)
	{
	}

	void setCgiPid(int pid)
	{
		_cgiPid = pid;
	}

	void setCgiPipes(int pipes[2])
	{
		_cgiPipes[0] = pipes[0];
		_cgiPipes[1] = pipes[1];
	}

	int getCgiPid()
	{
		return _cgiPid;
	}

	int *getCgiPipes()
	{
		return _cgiPipes;
	}

	HttpResponse &getRes()
	{
		return _res;
	}

	HttpRequest &getReq()
	{
		return _req;
	}
};

class CgiPollFd : public PollFd
{
private:
	HttpResponse &_res;
	ClientPollFd &_clientPollFd;
	int _pid;
	int _pipes[2];
	int _clientSocket;

public:
	CgiPollFd(int pid, int pipes[2], int clientSocket, HttpResponse &res, ClientPollFd &clientPollFd) : PollFd(pipes[0]),
																										_res(res),
																										_clientPollFd(clientPollFd)
	{
		_pid = pid;
		_pipes[0] = pipes[0];
		_pipes[1] = pipes[1];
		_clientSocket = clientSocket;
	}

	int getPid()
	{
		return _pid;
	}

	int *getipes()
	{
		return _pipes;
	}

	int getClientSocket()
	{
		return _clientSocket;
	}

	HttpResponse &getRes()
	{
		return _res;
	}

	ClientPollFd &getClient()
	{
		return _clientPollFd;
	}
};

#endif