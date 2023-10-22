#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include "webserv.hpp"

class HttpRequest
{
private:
	std::string _rawData;
	std::string _httpMethod;
	std::string _httpProtocol;
	std::string _url;
	std::string _body;
	std::string _hostName;

	std::vector<std::string> _headSplit;

	int _endOfRequestLinePos;

public:
	HttpRequest(){};

	std::string getHttpMethod()
	{
		return _httpMethod;
	}

	std::string getUrl()
	{
		return _url;
	}

	std::string getBody()
	{
		return _body;
	}

	std::string getRawData()
	{
		return _rawData;
	}

	std::string getHostName()
	{
		return _hostName;
	}

	int parseRequest(int clientSocket)
	{
		if (this->recvAll(clientSocket) == -1)
		{
			std::cerr << "Error while reading client socket\n";
			return 500;
		}

		this->splitHead();
		if (_headSplit.size() != 3 || _headSplit[1][0] != '/')
		{
			std::cerr << "Bad request syntax\n";
			return 400;
		}

		// RETURN 400 IF HTTP METHOD IS UNKNOWN!!
		_httpMethod = _headSplit[0];
		_url = this->replaceUrlPercent20(_headSplit[1]);
		compressSlashes(_url);
		_httpProtocol = _headSplit[2];

		_hostName = this->readHostName();

		this->readBody();
		return 200;
	}

	int recvAll(int clientSocket)
	{
		int readBytes = 1;
		int totalRead = 0;

		while (readBytes > 0)
		{
			_rawData.resize(totalRead + BUF_SIZE + 1, '\0');
			readBytes = recv(clientSocket, &_rawData[totalRead], BUF_SIZE, 0);
			totalRead += readBytes;
			// std::cout << readBytes << "\n";
		}

		_endOfRequestLinePos = _rawData.find(LINE_TERM);
		return (totalRead == 0 && readBytes == -1) ? -1 : 0;
		// return readBytes == -1 ? -1 : 0;
	}

	void splitHead()
	{
		int startPos = _rawData.find_first_not_of(" \t");
		int endPos = _rawData.find_first_of(" \t\r\n", startPos);

		while (startPos != std::string::npos && endPos != std::string::npos && startPos < _endOfRequestLinePos)
		{
			_headSplit.push_back(_rawData.substr(startPos, endPos - startPos));
			startPos = _rawData.find_first_not_of(" \t", endPos);
			endPos = _rawData.find_first_of(" \t\r\n", startPos);
		}
		// std::cout << _headSplit[0] << '\n';
		// std::cout << _headSplit[1] << '\n';
		// std::cout << _headSplit[2] << '\n';
	}

	std::string readHostName()
	{
		int namePos = _rawData.find("Host:");
		if (namePos == std::string::npos)
			return "";

		namePos += sizeof("Host: ") - 1;
		int endOfNamePos = _rawData.find_first_of(" :\t\r\n", namePos);
		return _rawData.substr(namePos, endOfNamePos - namePos);
	}

	std::string replaceUrlPercent20(std::string url)
	{
		int httpSpacePos = url.find("%20");
		while (httpSpacePos != std::string::npos)
		{
			url.replace(httpSpacePos, 3, " ");
			httpSpacePos = url.find("%20", httpSpacePos);
		}
		return url;
	}

	int readBody()
	{
		// WHAT TO DO WITH LAAARGE BODIES???
		int endOfHeaders = _rawData.find("\r\n\r\n");
		if (endOfHeaders != std::string::npos)
			_body = _rawData.substr(endOfHeaders, _rawData.size() - 1);
		return 0;
	}
};

#endif