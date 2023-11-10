#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include "webserv.hpp"
#include "StatusComments.hpp"
#include "MediaTypes.hpp"
#include "ServerStream.hpp"

class HttpResponse : public ServerStream
{
private:
	std::string _head;
	std::string _headers;
	std::string _permanentHeaders;
	std::string _body;

	bool _isSet;

public:
	HttpResponse()
	{
		_isSet = false;
	}

	void addHeader(std::string property, std::string value)
	{
		_permanentHeaders += property + ": " + value + LINE_TERM;
	}

	void reset()
	{
		_isSet = false;
		_head = "";
		_headers = "";
		_permanentHeaders = "";
		_body = "";
		_rawData = "";
		_outputData = "";
	}

	void set(int statusCode, std::string path = "", std::string *body = NULL)
	{
		_isSet = true;

		this->createHead(statusCode);

		if (path != "")
		{
			_headers = "Content-Type: " + MediaTypes::getType(path) + LINE_TERM;
			_headers += "Content-Length: " + std::to_string(body->size()) + LINE_TERM;
			_body = *body;
		}
	}

	void createHead(int statusCode)
	{
		_head = "HTTP/1.1 " + std::to_string(statusCode) + " " + StatusComments::get(statusCode) + LINE_TERM;
	}

	void loadFile(int serverStatusCode, std::string path)
	{
		this->reset();
		int accessStatus = checkFileAccess(path);
		if (accessStatus != 200)
			throw accessStatus;

		std::string fileContent;
		if (getFileContent(path, fileContent))
		{
			std::cerr << "Error while reading " << path << "\n";
			throw 500;
		}
		this->set(serverStatusCode, path, &fileContent);
	}

	void updateOutputData()
	{
		if (_rawData != "")
		{
			_outputData = _head + _headers + _permanentHeaders + _rawData + LINE_TERM;
		}
		else
		{
			_outputData = _head + _headers + _permanentHeaders + LINE_TERM + _body + LINE_TERM;
		}
	}

	bool isSet()
	{
		return _isSet;
	}
};

#endif