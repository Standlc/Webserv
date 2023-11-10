#include "webserv.hpp"
#include "Server.hpp"

std::string getSocketPort(int socket)
{
	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);

	if (getsockname(socket, (struct sockaddr *)&sin, &len) == -1)
	{
		std::cerr << "getsockname: " << strerror(errno) << "\n";
		throw 500;
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

int createBindedSocket(struct addrinfo *addrInfo)
{
	int socketFd = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
	if (socketFd == -1)
	{
		std::cerr << "socket: " << strerror(errno) << "\n";
		return -1;
	}
	// fcntl(socketFd, F_SETFL, O_NONBLOCK);
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

	if (dotPos == -1 || dotPos < lastSlashPos)
		return "undefined";
	return fileName.substr(dotPos);
}

int checkFileAccess(std::string path)
{
	if (access(path.c_str(), F_OK) == -1)
		return 404;
	if (access(path.c_str(), R_OK) == -1)
		return 403;
	return 200;
}

void tryGetIndexes(std::string newPath, LocationBlock &block, HttpResponse &res)
{
	int locationBlockIndexSize = block.indexFiles.size();
	for (int i = 0; i < locationBlockIndexSize; i++)
	{
		try
		{
			res.loadFile(200, newPath + "/" + block.indexFiles[i]);
			return;
		}
		catch (int status)
		{
			if (status != 404 && status != 403)
				throw;
		}
	}
	throw 403;
}

void getMethod(LocationBlock &block, HttpRequest &req, HttpResponse &res)
{
	std::string completePath = block.getCompletePath(req.getUrl());

	int accessStatus = checkFileAccess(completePath);
	if (accessStatus != 200)
		throw accessStatus;

	if (!isDirectory(completePath))
	{
		res.loadFile(200, completePath);
		return;
	}

	try
	{
		tryGetIndexes(completePath, block, res);
	}
	catch (int status)
	{
		if (block.autoIndex == false)
			throw;
		block.listFiles(res, req.getUrl());
	}
}

void postMethod(LocationBlock &block, HttpRequest &req, HttpResponse &res)
{
	std::string completePath = block.getCompletePath(req.getUrl());

	if (completePath.back() == '/')
		throw 403;

	if (checkFileAccess(completePath) != 404)
		throw 409;

	std::ofstream file(completePath.c_str());
	if (!file)
		throw 500;

	// Extract request body???
	file << req.getBody().c_str();
	file.close();

	res.loadFile(200, completePath);
}

void deleteMethod(LocationBlock &block, HttpRequest &req, HttpResponse &res)
{
	std::string completePath = block.getCompletePath(req.getUrl());

	std::cout << "DELETE\n";

	if (checkFileAccess(completePath) == 404)
		throw 404;
	
	if (isDirectory(completePath))
		throw 403;

	if (std::remove(completePath.c_str()) != 0)
		throw 500;

	res.loadFile(303, "defaultPages/delete_success.html");
}

// TO DO:
// body max size
// cgi conig
// make members private

int main(int argc, char *argv[])
{
	// if (std::remove("www/folder") != 0)
	// 	std::cout << "404";
	// return 0;
	// std::cout << getPathInfo("/truc/truc/api", "/truc/truc") << "\n";
	// std::cout << getPathInfo("/truc/truc/api", "/truc/truc/") << "\n";
	// std::cout << getPathInfo("/truc/truc/api", "/truc/truc/api") << "\n";
	// std::cout << getPathInfo("/truc/truc/api", "/truc/truc/apit") << "\n";
	// return 0;

	// std::cout << checkFileAccess("/dgnd") << "\n";
	// std::cout << checkFileAccess("www/") << "\n";
	// std::cout << checkFileAccess("www/index.html") << "\n";
	// std::cout << checkFileAccess("www/index.html/") << "\n";
	// std::ofstream file("test/");
	// // file << "hello\r\n";
	// file.close();
	// return 0;

	Server server;

	server.blocks.resize(1);

	server.blocks[0].port = "3000";
	server.blocks[0].root = "www";
	server.blocks[0].isDefault = true;
	server.blocks[0].indexFiles.push_back("index.html");
	server.blocks[0].errorFiles[404] = "/404.html";

	// server.blocks[1].port = "5000";
	// server.blocks[1].root = "www2";
	// server.blocks[1].isDefault = true;
	// server.blocks[1].indexFiles.push_back("index");
	// server.blocks[1].indexFiles.push_back("index.html");
	// server.blocks[1].errorFiles[404] = "/404.html";

	// server.blocks[2].port = "3000";
	// server.blocks[2].root = "www2";
	// server.blocks[2].isDefault = false;
	// server.blocks[2].hostNames.push_back("virtual.com");
	// server.blocks[2].indexFiles.push_back("index.html");

	if (server.listen() == -1)
		return 1;

	server.blocks[0]._locationBlocks.resize(4);

	server.blocks[0]._locationBlocks[0].path = "/";
	server.blocks[0]._locationBlocks[0].isExact = false;
	server.blocks[0]._locationBlocks[0].handlers["GET"] = getMethod;
	server.blocks[0]._locationBlocks[0].inheritServerBlock(server.blocks[0]);

	server.blocks[0]._locationBlocks[1].path = "/folder/";
	server.blocks[0]._locationBlocks[1].indexFiles.push_back("/index");
	server.blocks[0]._locationBlocks[1].isExact = false;
	server.blocks[0]._locationBlocks[1].autoIndex = true;
	server.blocks[0]._locationBlocks[1].handlers["GET"] = getMethod;
	server.blocks[0]._locationBlocks[1].handlers["POST"] = postMethod;
	server.blocks[0]._locationBlocks[1].handlers["DELETE"] = deleteMethod;
	server.blocks[0]._locationBlocks[1].inheritServerBlock(server.blocks[0]);

	server.blocks[0]._locationBlocks[2].path = "/api/truc";
	server.blocks[0]._locationBlocks[2].redirection.url = "/";
	server.blocks[0]._locationBlocks[2].redirection.statusCode = 301;
	server.blocks[0]._locationBlocks[2].isExact = false;
	server.blocks[0]._locationBlocks[2].handlers["GET"] = getMethod;
	server.blocks[0]._locationBlocks[2].inheritServerBlock(server.blocks[0]);

	server.blocks[0]._locationBlocks[3].path = "/upload";
	server.blocks[0]._locationBlocks[3].isExact = false;
	server.blocks[0]._locationBlocks[3].handlers["GET"] = getMethod;
	server.blocks[0]._locationBlocks[3].handlers["POST"] = postMethod;
	server.blocks[0]._locationBlocks[3].cgiExtensions[".py"] = "/usr/bin/python3";
	server.blocks[0]._locationBlocks[3].inheritServerBlock(server.blocks[0]);

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
