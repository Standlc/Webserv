#include "../parsing.hpp"

int	check_proxy_pass(const string &content)
{
	int i = 11;

	if (!check_no_backspace(content))
		return (error_message(line_tracker(content.substr(i)), MPROXY, EXPROXY));
	string url = content.substr(i, content.find(';') - i);
	if (strncmp(url.c_str(), "http://", strlen("http://")))
		return (error_message(line_tracker(content.substr(i)), MPROXY, EXPROXY));
	return (url.size() + i + 1);
}

int check_methods(const string &content)
{
	int i = 8;
	while (content[i] != ';')
	{
		if (!strncmp(&content[i], "GET", strlen("GET")))
			i += 3;
		else if (!strncmp(&content[i], "POST", strlen("POST")))
			i += 4;
		else if (!strncmp(&content[i], "DELETE", strlen("DELETE")))
			i += 6;
		else if (!strncmp(&content[i], "PUT", strlen("PUT")))
			i += 3;
		else if (!strncmp(&content[i], "TRACE", strlen("TRACE")))
			i += 5;
		else if (!strncmp(&content[i], "CONNECT", strlen("CONNECT")))
			i += 7;
		else if (!strncmp(&content[i], "HEAD", strlen("HEAD")))
			i += 4;
		else if (!strncmp(&content[i], "OPTION", strlen("OPTION")))
			i += 6;
		else if (!strncmp(&content[i], "PATCH", strlen("PATCH")))
			i += 5;
		else
			return (error_message(line_tracker(content.substr(i)), MMETHOD, NOEX));
		if (content[i] == ',')
			i++;
		else if (content[i] == ';')
			;
		else
			return (error_message(line_tracker(content.substr(i)), MMETHOD, NOEX));
	}
	return (i + 1);
}

int check_redirect(const string &content)
{
	int i = 9;

	if (!check_no_backspace(content))
		return (error_message(line_tracker(content.substr(i)), MREDIRECT, EXREDIRECT));
	int number = atoi(&content[i]);
	if (!((number >= 300 && number <= 304) || (number == 307 || number == 308)))
		return (error_message(line_tracker(content.substr(i)), MREDIRECT, EXREDIRECT));
	while (content[i] >= '0' && content[i] <= '9')
		i++;
	if (content[i] != ',')
		return (error_message(line_tracker(content.substr(i)), MSYNT, NOEX));
	i++;
	string root = content.substr(i, next_semicolon(&content[i]));
	if (root == "")
		return (error_message(line_tracker(content.substr(i)), MREDIRECT, EXREDIRECT));
	i += root.size();
	return (i + 1);
}

int check_location(const string &content)
{
	int i = 9;

	string path = content.substr(i, content.find('{') - i);
	if (path == "")
		return (error_message(line_tracker(content.substr(i)), MLOCFOL, NOEX));
	if (path == "/")
		required["location"] = 1;
	i += path.size();
	if (content[i] != '{')
		return (error_message(line_tracker(content), MBRACK, NOEX));
	i++;
	while (content[i] != '}' && content[i])
	{
		i += check_block(content.substr(i));
		if (error_parsing == ERR)
			return (ERR);
	}
	if (!content[i])
		return (error_message(line_tracker(content.substr(i)), MMETHOD, NOEX));
	return (i + 1);
}