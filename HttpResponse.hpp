#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include "webserv.hpp"

class HttpResponse
{
private:
	std::string _response;
	std::string _head;
	std::string _headers;
	std::string _body;
	static MediaTypes mediaTypes;

public:
	HttpResponse() {}

	void addHeader(std::string property, std::string value)
	{
		_headers += property + ": " + value + LINE_TERM;
		this->updateResponse();
	}

	void set(int statusCode, std::string statusComment, std::string path, std::string &body)
	{
		_head = "HTTP/1.1 " + std::to_string(statusCode) + " " + statusComment + LINE_TERM;

		this->addHeader("Content-Type", mediaTypes.getType(path));
		this->addHeader("Content-Length", std::to_string(body.size()));

		_body = body + LINE_TERM;

		this->updateResponse();
	}

	int loadFile(int serverStatusCode, std::string path, std::string comment)
	{
		int statusCode = checkFileAccess(path);
		if (statusCode != 200)
			return statusCode;

		std::string fileContent;
		if (getFileContent(path, fileContent))
		{
			std::cerr << "Error while reading " << path << "\n";
			return 500;
		}
		this->set(serverStatusCode, comment, path, fileContent);
		return 200;
	}

	int sendAll(int socket)
	{
		int responseSize = _response.size();
		int sentBytes = send(socket, &_response[0], responseSize, 0);
		int totalSentBytes = sentBytes;

		// std::cout << "Sending...\n";
		while (sentBytes != -1 && totalSentBytes < responseSize)
		{
			sentBytes = send(socket, &_response[totalSentBytes], responseSize - totalSentBytes, 0);
			totalSentBytes += sentBytes;
			// std::cout << sentBytes << "\n";
		}
		// std::cout << "finished sending response\n";
		return (sentBytes == -1) ? -1 : 0;
	}

	void updateResponse()
	{
		_response = _head + _headers + LINE_TERM + _body;
	}

	std::string getResponse()
	{
		// return _head + _headers + LINE_TERM + _body;
		return _response;
	}
};

MediaTypes HttpResponse::mediaTypes;

#endif