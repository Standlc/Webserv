#ifndef SERVER_BLOCK
#define SERVER_BLOCK

#include "webserv.hpp"

class ServerBlock
{
private:
	std::map<std::string, methods> _httpHandlers;

public:
	bool isDefault;
	std::string root;
	std::string port;

	std::vector<std::string> hostNames;
	std::vector<std::string> indexFiles;

	void setHttpMethod(std::string httpMethod, methodHandler handler, std::string path = "")
	{
		if (path == "")
			path = "/";
		_httpHandlers[httpMethod][path] = handler;
	}

	std::string execute(HttpRequest &req, HttpResponse &res)
	{
		if (!this->has(req.getHttpMethod()))
			return "501";

		if (_httpHandlers[req.getHttpMethod()].count(req.getUrl()))
			return _httpHandlers[req.getHttpMethod()][req.getUrl()](*this, req, res);

		if (_httpHandlers[req.getHttpMethod()].count("/"))
			return _httpHandlers[req.getHttpMethod()]["/"](*this, req, res);

		return "404";
	}

	std::string getCompleteReqPath(std::string path)
	{
		// IF RELATIVE PATH
		// 	PATH_TO_CONFIG_FILE + ROOT_PATH

		// HANDLE MULTIPLE INDEX FILES???
		if (path == "/")
			return root + path + indexFiles[0];
		return root + path;
	}

	bool has(std::string httpMethod)
	{
		return _httpHandlers.find(httpMethod) != _httpHandlers.end();
	}
};

#endif