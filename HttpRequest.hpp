#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include "webserv.hpp"
#include "ServerStream.hpp"

void compressSlashes(std::string &str)
{
	int startSlashPos = str.find_first_of("/");
	int endSlashPos = str.find_first_not_of("/", startSlashPos);

	while (startSlashPos != -1)
	{
		str.replace(startSlashPos, endSlashPos != -1 ? endSlashPos - startSlashPos : -1, "/");
		startSlashPos = str.find_first_of("/", startSlashPos + 1);
		endSlashPos = str.find_first_not_of("/", startSlashPos);
	}
}

void uriDecode(std::string &url, std::string find, char replace)
{
	int pos = url.find(find);

	while (pos != -1)
	{
		url.replace(pos, 3, &replace);
		pos = url.find(find, pos + 1);
	}
}
// ␣	!	"	#	$	%	&	'	(	)	*	+	,	/	:	;	=	?	@	[	]
// %20	%21	%22	%23	%24	%25	%26	%27	%28	%29	%2A	%2B	%2C	%2F	%3A	%3B	%3D	%3F	%40	%5B	%5D
void percentDecode(std::string &url)
{
	uriDecode(url, "%20", ' ');
	uriDecode(url, "%21", '!');
	uriDecode(url, "%22", '"');
	uriDecode(url, "%23", '#');
	uriDecode(url, "%24", '$');
	uriDecode(url, "%25", '%');
	uriDecode(url, "%26", '&');
	uriDecode(url, "%27", '\'');
	uriDecode(url, "%28", '(');
	uriDecode(url, "%29", ')');

	uriDecode(url, "%2A", '*');
	uriDecode(url, "%2B", '+');
	uriDecode(url, "%2C", ',');
	uriDecode(url, "%2F", '/');

	uriDecode(url, "%3A", ':');
	uriDecode(url, "%3B", ';');
	uriDecode(url, "%3D", '=');
	uriDecode(url, "%3F", '?');

	uriDecode(url, "%40", '@');

	uriDecode(url, "%5B", '[');	
	uriDecode(url, "%5D", ']');	
}

class HttpRequest : public ServerStream
{
private:
	std::string _httpMethod;
	std::string _httpProtocol;
	std::string _url;
	std::string _body;

public:
	HttpRequest()
	{
	};

	std::string getHttpMethod()
	{
		return _httpMethod;
	}

	std::string getProtocol()
	{
		return _httpProtocol;
	}

	std::string getUrl()
	{
		return _url;
	}

	std::string &getBody()
	{
		return _body;
	}

	void parseRequest()
	{
		this->parseHead();
		this->readBody();
	}

	void parseHead()
	{
		const std::vector<std::string> &split = this->splitHead();
		if (split.size() != 3)
			throw 400;

		_httpMethod = split[0];
		_url = split[1];
		compressSlashes(_url);
		percentDecode(_url);
		_httpProtocol = split[2];
	}

	std::vector<std::string> splitHead()
	{
		std::vector<std::string> split;
		std::string head = _rawData.substr(0, _rawData.find(LINE_TERM));

		int startPos = head.find_first_not_of(" \t");

		while (startPos != -1)
		{
			int endPos = head.find_first_of(" \t\r", startPos);
			split.push_back(head.substr(startPos, endPos - startPos));
			startPos = head.find_first_not_of(" \t", endPos);
		}

		// std::cout << split[0] << '\n';
		// std::cout << split[1] << '\n';
		// std::cout << split[2] << '\n';
		return split;
	}

	std::string getHeader(std::string attribute)
	{
		std::string fullHeader = "\r\n" + attribute + ": ";
		int attrPos = _rawData.find(fullHeader);
		if (attrPos == -1)
			return "";

		attrPos += fullHeader.length();
		int endOfattrPos = _rawData.find_first_of(" \t\r", attrPos);
		return _rawData.substr(attrPos, endOfattrPos - attrPos);
	}

	void readBody()
	{
		int endOfHeaders = _rawData.find("\r\n\r\n");
		if (endOfHeaders != -1)
			_body = _rawData.substr(endOfHeaders + 4);
	}

	void updateOutputData()
	{
		_outputData = _rawData;
	}
};

#endif