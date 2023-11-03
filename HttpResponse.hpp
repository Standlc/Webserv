#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include "webserv.hpp"

class HttpResponse
{
private:
	std::string _response;
	std::string _head;
	std::string _headers;
	std::string _permanentHeaders;
	std::string _body;

public:
	HttpResponse() {}

	void addHeader(std::string property, std::string value)
	{
		_permanentHeaders += property + ": " + value + LINE_TERM;
	}

	void set(int statusCode, std::string statusComment, std::string path, std::string &body)
	{
		_head = "HTTP/1.1 " + std::to_string(statusCode) + " " + statusComment + LINE_TERM;

		_headers = "Content-Type: " + MediaTypes::getType(path) + LINE_TERM;
		_headers += "Content-Length: " + std::to_string(body.size()) + LINE_TERM;

		_body = body + LINE_TERM;
	}

	void loadFile(int serverStatusCode, std::string path)
	{
		checkFileAccess(path);

		std::string fileContent;
		if (getFileContent(path, fileContent))
		{
			std::cerr << "Error while reading " << path << "\n";
			throw 500;
		}
		this->set(serverStatusCode, StatusComments::get(serverStatusCode), path, fileContent);
	}

	int sendAll(int socket)
	{
		updateResponse();

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
		_response = _head + _headers + _permanentHeaders + LINE_TERM + _body;
	}

	std::string getResponse()
	{
		updateResponse();
		return _response;
	}
};

// MediaTypes HttpResponse::mediaTypes;

#endif