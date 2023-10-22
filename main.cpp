#include "webserv.hpp"

void compressSlashes(std::string &str)
{
	int startSlashPos = str.find_first_of("/");
	int endSlashPos = str.find_first_not_of("/", startSlashPos);

	while (startSlashPos != -1)
	{
		// std::cout << startSlashPos << " " << endSlashPos << "\n";
		str.replace(startSlashPos, endSlashPos != -1 ? endSlashPos - startSlashPos : -1, "/");
		startSlashPos = str.find_first_of("/", endSlashPos);
		endSlashPos = str.find_first_not_of("/", startSlashPos);
	}
}

std::string getSocketPort(int socket)
{
	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);

	if (getsockname(socket, (struct sockaddr *)&sin, &len) == -1)
	{
		std::cerr << "getsockname: " << strerror(errno) << "\n";
		return "";
	}
	// printf("port number %d\n", ntohs(sin.sin_port));
	return std::to_string(ntohs(sin.sin_port));
}

struct addrinfo *getOwnAddressInfo(const char *port)
{
	struct addrinfo *res;
	struct addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;	 // AF_INET or AF_INET6 to force version
	hints.ai_socktype = SOCK_STREAM; // TCP or UDP (datagram)
	hints.ai_flags = AI_PASSIVE;	 // assign the address of my local host to the socket structures

	int status = getaddrinfo(NULL, port, &hints, &res);
	if (status == -1)
	{
		std::cerr << "getaddrinfo: " << gai_strerror(errno) << "\n";
		return NULL;
	}
	return res;
}

int createBindedNonBlockingSocket(struct addrinfo *addrInfo)
{
	int socketFd = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
	if (socketFd == -1)
	{
		std::cerr << "socket: " << strerror(errno) << "\n";
		return -1;
	}
	fcntl(socketFd, F_SETFL, O_NONBLOCK);
	return socketFd;
}

int bindSocket(int socketFd, struct addrinfo *addrInfo)
{
	int yes = 1;
	if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
	{
		std::cerr << "setsockopt: " << strerror(errno) << "\n";
		return -1;
	}
	int status = bind(socketFd, addrInfo->ai_addr, addrInfo->ai_addrlen);
	if (status == -1)
	{
		std::cerr << "bind: " << strerror(errno) << "\n";
		return -1;
	}
	return 0;
}

int listenToSocket(int socketFd, std::string port)
{
	int status = listen(socketFd, 20);
	if (status == -1)
	{
		std::cerr << "listen: " << strerror(errno) << "\n";
		return -1;
	}
	std::cout << "Server is listening on port " << port << "..."
			  << "\n";
	return 0;
}

int getFileContent(std::string path, std::string &buf)
{
	std::ifstream file(path);
	if (!file)
		return 1;

	std::stringstream fileContent;
	fileContent << file.rdbuf();
	buf = fileContent.str();
	file.close();
	return 0;
}

std::string getFileExtension(std::string fileName)
{
	int lastSlashPos = fileName.find_last_of('/');
	int dotPos = fileName.find_last_of(".");

	if (dotPos == std::string::npos || dotPos < lastSlashPos)
		return "undefined";
	return fileName.substr(dotPos, std::string::npos);
}

int checkFileAccess(std::string path)
{
	if (access(path.c_str(), F_OK) == -1)
		return 404;
	if (access(path.c_str(), R_OK) == -1)
		return 403;
	return 200;
}

int getMethod(LocationBlock &block, HttpRequest &req, HttpResponse &res)
{
	std::string newPath = block.root + req.getUrl();

	int statusCode = checkFileAccess(newPath);
	if (statusCode != 200)
		return statusCode;

	int isDir = isDirectory(newPath);
	if (isDir == -1)
		return 500;
	if (!isDir)
		return res.loadFile(200, newPath, "OK");

	for (int i = 0; i < block.indexFiles.size(); i++)
	{
		newPath = newPath + "/" + block.indexFiles[i];
		statusCode = res.loadFile(200, newPath, "OK");

		if (statusCode != 404)
			return statusCode;
	}

	if (!block.autoIndex)
		return 403;

	return block.listFiles(res, req.getUrl());
}

int getApi(LocationBlock &block, HttpRequest &req, HttpResponse &res)
{
	std::string body = "Welcome to the API";
	res.set(200, "OK", ".txt", body);
	return 200;
}

int postMethod(LocationBlock &block, HttpRequest &req, HttpResponse &res)
{
	return 200;
}

int deleteMethod(LocationBlock &block, HttpRequest &req, HttpResponse &res)
{
	return 200;
}

int main(int argc, char *argv[])
{
	Server server;

	server.blocks.resize(2);

	server.blocks[0].port = "3000";
	server.blocks[0].root = "www";
	server.blocks[0].isDefault = true;
	server.blocks[0].indexFiles.push_back("index.html");
	server.blocks[0].errorFiles[404] = "/404.html";

	server.blocks[1].port = "5000";
	server.blocks[1].root = "www2";
	server.blocks[1].isDefault = true;
	server.blocks[1].indexFiles.push_back("index");
	server.blocks[1].indexFiles.push_back("index.html");
	server.blocks[1].errorFiles[404] = "/404.html";

	// server.blocks[2].port = "3000";
	// server.blocks[2].root = "www2";
	// server.blocks[2].isDefault = false;
	// server.blocks[2].hostNames.push_back("virtual.com");
	// server.blocks[2].indexFiles.push_back("index.html");

	if (server.listen() == -1)
		return 1;

	server.blocks[0]._locationBlocks.resize(3);

	server.blocks[0]._locationBlocks[0].path = "/";
	server.blocks[0]._locationBlocks[0].isExact = false;
	server.blocks[0]._locationBlocks[0].handlers["GET"] = getMethod;
	server.blocks[0]._locationBlocks[0].inheritServerBlock(server.blocks[0]);

	server.blocks[0]._locationBlocks[1].path = "/folder/";
	server.blocks[0]._locationBlocks[1].indexFiles.push_back("/index");
	server.blocks[0]._locationBlocks[1].isExact = false;
	server.blocks[0]._locationBlocks[1].autoIndex = true;
	server.blocks[0]._locationBlocks[1].handlers["GET"] = getMethod;
	server.blocks[0]._locationBlocks[1].inheritServerBlock(server.blocks[0]);

	server.blocks[0]._locationBlocks[2].path = "/api/truc";
	server.blocks[0]._locationBlocks[2].isExact = false;
	server.blocks[0]._locationBlocks[2].handlers["GET"] = getApi;

	//

	server.blocks[1]._locationBlocks.resize(1);

	server.blocks[1]._locationBlocks[0].path = "/";
	server.blocks[1]._locationBlocks[0].isExact = false;
	server.blocks[1]._locationBlocks[0].handlers["GET"] = getMethod;
	server.blocks[1]._locationBlocks[0].inheritServerBlock(server.blocks[1]);

	// block.path = "/";
	// block.isExact = false;
	// block.handlers["GET"] = getMethod;
	// server.blocks[0].addpath(block);

	// server.blocks[0].addpath(block);

	// server.blocks[0].http["GET"]
	// server.blocks[0].setpath("GET", "/", getMethod);

	// server.blocks[0].setpath("GET", "/api", getApi);

	// server.blocks[0].setpath("POST", "/", postMethod);

	//

	// server.blocks[1].setpath("GET", "/", getMethod);

	//

	// server.blocks[2].setpath("GET", "/", getMethod);

	if (server.monitorClients() == -1)
		return 1;
	return 0;
}

// char ipstr[INET6_ADDRSTRLEN];
// for (p = res; p != NULL; p = p->ai_next)
// {
// 	void *addr;
// 	if (p->ai_family == AF_INET)
// 	{ // IPv4
// 		struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
// 		addr = &(ipv4->sin_addr);
// 	}
// 	else
// 	{ // IPv6
// 		struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
// 		addr = &(ipv6->sin6_addr);
// 	}
// 	inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
