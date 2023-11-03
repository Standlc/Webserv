#include "webserv.hpp"

void compressSlashes(std::string &str)
{
	int startSlashPos = str.find_first_of("/");
	int endSlashPos = str.find_first_not_of("/", startSlashPos);

	// std::cout << str << "\n";
	while (startSlashPos != -1)
	{
		// std::cout << startSlashPos << " " << endSlashPos << "\n";
		str.replace(startSlashPos, endSlashPos != -1 ? endSlashPos - startSlashPos : -1, "/");
		startSlashPos = str.find_first_of("/", startSlashPos + 1);
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

void tryGetIndexes(std::string newPath, LocationBlock &block, HttpResponse &res)
{
	for (int i = 0; i < block.indexFiles.size(); i++)
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

std::string getPathInfo(std::string reqUrl, std::string locationUrl)
{
	int locationUrlLen = locationUrl.length();
	if (locationUrlLen > reqUrl.length())
		return "";

	int lastSlashPos = reqUrl.substr(locationUrlLen).find_last_of("/");
	if (lastSlashPos == -1)
		return reqUrl.substr(locationUrlLen);

	return reqUrl.substr(lastSlashPos + locationUrlLen + 1);
}

void postMethod(LocationBlock &block, HttpRequest &req, HttpResponse &res)
{
	std::string fileName = getPathInfo(req.getUrl(), block.path);
	std::string completePath = block.getCompletePath(block.path + "/" + fileName);

	// std::cout << completePath << " " << block.path << "\n";
	int accessStatus = checkFileAccess(completePath);

	if (fileName.back() == '/')
		throw 400;

	if (accessStatus != 404)
		throw 409;

	std::ofstream file(completePath);
	if (!file)
		throw 500;

	file << req.getBody().c_str();
	file.close();

	res.loadFile(200, completePath);
}

void deleteMethod(LocationBlock &block, HttpRequest &req, HttpResponse &res)
{
}

int main(int argc, char *argv[])
{

	// std::cout << getPathInfo("/truc/truc/api", "/truc/truc") << "\n";
	// std::cout << getPathInfo("/truc/truc/api", "/truc/truc/") << "\n";
	// std::cout << getPathInfo("/truc/truc/api", "/truc/truc/api") << "\n";
	// std::cout << getPathInfo("/truc/truc/api", "/truc/truc/apit") << "\n";
	// return 0;

	// std::cout << checkFileAccess("/dgnd") << "\n";
	// std::cout << checkFileAccess("www/") << "\n";
	// std::cout << checkFileAccess("www/index.html") << "\n";
	// std::cout << checkFileAccess("www/index.html/") << "\n";
	// std::ofstream file("test");
	// file << "hello\r\n";
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
	server.blocks[0]._locationBlocks[1].handlers["POST"] = postMethod;
	server.blocks[0]._locationBlocks[1].inheritServerBlock(server.blocks[0]);

	server.blocks[0]._locationBlocks[2].path = "/api/truc";
	server.blocks[0]._locationBlocks[2].redirection.url = "/";
	server.blocks[0]._locationBlocks[2].redirection.statusCode = 301;
	server.blocks[0]._locationBlocks[2].isExact = false;
	server.blocks[0]._locationBlocks[2].handlers["GET"] = getMethod;
	server.blocks[0]._locationBlocks[2].inheritServerBlock(server.blocks[0]);

	//

	// server.blocks[1]._locationBlocks.resize(1);

	// server.blocks[1]._locationBlocks[0].path = "/";
	// server.blocks[1]._locationBlocks[0].isExact = false;
	// server.blocks[1]._locationBlocks[0].handlers["GET"] = getMethod;
	// server.blocks[1]._locationBlocks[0].inheritServerBlock(server.blocks[1]);

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
