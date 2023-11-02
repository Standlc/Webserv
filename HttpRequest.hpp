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
	std::string _hostPort;

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

	std::string getHostPort()
	{
		return _hostPort;
	}

	void parseRequest(int clientSocket)
	{
		if (this->recvAll(clientSocket) == -1)
		{
			std::cerr << "Error while reading client socket\n";
			throw 500;
		}

		this->splitHead();
		if (_headSplit.size() != 3 || _headSplit[1][0] != '/')
		{
			std::cerr << "Bad request syntax\n";
			throw 400;
		}

		// RETURN 400 IF HTTP METHOD IS UNKNOWN!!

		_httpMethod = _headSplit[0];
		_url = this->replaceUrlPercent20(_headSplit[1]);
		compressSlashes(_url);
		_httpProtocol = _headSplit[2];
		this->readHostName();
		this->readBody();
	}

	int recvAll(int clientSocket)
	{
		int readBytes = 0;
		int totalRead = 0;

		do
		{
			totalRead += readBytes;
			_rawData.resize(totalRead + BUF_SIZE + 1, '\0');
			readBytes = recv(clientSocket, &_rawData[totalRead], BUF_SIZE, 0);
			// std::cout << readBytes << "\n";
		} while (readBytes > 0);

		_endOfRequestLinePos = _rawData.find(LINE_TERM);
		return (readBytes == -1 && totalRead == 0) ? -1 : 0;
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

	void readHostName()
	{
		int namePos = _rawData.find("\r\nHost: ");
		if (namePos == std::string::npos)
			return;

		namePos += sizeof("\r\nHost: ") - 1;
		int endOfNamePos = _rawData.find_first_of(" :\t\r\n", namePos);
		_hostName = _rawData.substr(namePos, endOfNamePos - namePos);

		int endOfPortPos = _rawData.find_first_of(" \t\r\n", endOfNamePos);
		_hostPort = _rawData.substr(endOfNamePos + 1, endOfPortPos - (endOfNamePos + 1));
		// std::cout << _hostPort << "\n\n";
	}

	std::string replaceUrlPercent20(std::string url)
	{
		int httpSpacePos = url.find("%20");
		while (httpSpacePos != std::string::npos)
		{
			url.replace(httpSpacePos, 3, " ");
			httpSpacePos = url.find("%20", httpSpacePos + 1);
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