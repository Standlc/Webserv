#include "parsing.hpp"

int fill_location_auto_index(const string &file, LocationBlock &location)
{
	int AutoIndex = false;
	if (!strncmp(file.c_str(), "on", strlen("on")))
		AutoIndex = true;
	location.setAutoIndex(AutoIndex);
	return (file.find(';') + 1);
}

int fill_cgi_extensions(const string &file, LocationBlock &location)
{
	int i = 1;

	while (file[i] != '}')
	{
		String extention = file.substr(i, file.find(':', i) - i);
		i = file.find(':', i);
		i++;
		String binarie = file.substr(i, file.find(';', i) - i);
		location.addCgiCommand(extention, binarie);
		while (file[i] != ';')
			i++;
		i++;
	}
	return (i + 1);
}

int fill_add_header(const string &file, LocationBlock &location)
{
	int i = 1;

	while (file[i] != '}')
	{
		String key = file.substr(i, file.find(':', i) - i);
		i = file.find(':', i);
		i++;
		String value = file.substr(i, file.find(';', i) - i);
		location.addHeader(key, value);
		while (file[i] != ';')
			i++;
		i++;
	}
	return (i + 1);
}

int fill_methods(const string &file, LocationBlock &location)
{
	int i = 0;

	String methods = file.substr(0, file.find(';'));
	while (methods[i])
	{
		if (methods[i] == ',')
			i++;
		String method[] = {methods.substr(i, methods.find(',', i) - i), ""};
    	location.setAllowedMethods(method);
		i += method[0].size();
	}
	return (i + 1);
}

int fill_body_max_size(const string &file, Block &location)
{
	int i = 0;

	size_t number = atoi(file.c_str());
	while (isdigit(file[i]))
		i++;
	if (!strncmp(file.substr(i, file.find(';') - i).c_str(), "MB", strlen("MB")))
		number *= 1000000;
	location.setMaxBodySize(number);
	return (i + 1);
}



int fill_cookie(const string &file, Block &server)
{
	server.addSessionCookie(file.substr(0, file.find(';')));
	return (file.find(';') + 1);
}

int fill_port(const string &file, ServerBlock &server)
{
	server.set("", file.substr(0, file.find(';')), true); // a ajouter
	return (file.find(';') + 1);
}

int fill_host_name(const string &file, ServerBlock &server)
{
	int end = 0;
	int start = 0;

	while (file[start] != ';')
	{
		if (file[start] == ',')
			start++;
		server.addHostName(file.substr(start, file.find_first_of(",;", start) - start));
		start = file.find_first_of(",;", start);
	}
	return (file.find(';') + 1);
}

int fill_index_file(const string &file, Block &server)
{
	server.setIndex(file.substr(0, file.find(';')));
	return (file.find(';') + 1);
}

int fill_upload_root(const string &file, Block &server)
{
	server.setUploadRoot(file.substr(0, file.find(';')));
	return (file.find(';') + 1);
}

int fill_root(const string &file, Block &server)
{
	server.setRoot(file.substr(0, file.find(';')));
	return (file.find(';') + 1);
}

int fill_proxy_pass(const string &file, LocationBlock &server)
{
	server.setProxyPass(file.substr(0, file.find(';')));
	return (file.find(';') + 1);
}

int fill_error_pages(const string &file, Block &server)
{
	int i = 1;

	while (file[i] != '}')
	{
		int number = atoi(&file[i]);
		while (file[i] != ':')
			i++;
		i++;
		String file_error = file.substr(i, file.find(';', i) - i);
		server.addErrorPage(number, file_error);
		while (file[i] != ';')
			i++;
		i++;
	}
	return (i + 1);
}

int	found_location_data(const string &file, LocationBlock &location)
{
	if (!strncmp(file.c_str(), "root:", strlen("root:")))
		return (fill_root(file.substr(5), location) + 5);
	if (!strncmp(file.c_str(), "index_file:", strlen("index_file:")))
		return (fill_index_file(file.substr(11), location) + 11);
	if (!strncmp(file.c_str(), "error_pages:", strlen("error_pages:")))
		return (fill_error_pages(file.substr(12), location) + 12);
	if (!strncmp(file.c_str(), "cgi_extensions:", strlen("cgi_extensions:")))
		return (fill_cgi_extensions(file.substr(15), location) + 15);
	if (!strncmp(file.c_str(), "auto_index:", strlen("auto_index:")))
		return (fill_location_auto_index(file.substr(11), location) + 11);
	if (!strncmp(file.c_str(), "add_headers:", strlen("add_headers:")))
		return (fill_add_header(file.substr(12), location) + 12);
	if (!strncmp(file.c_str(), "methods:", strlen("methods:")))
		return (fill_methods(file.substr(8), location) + 8);
	if (!strncmp(file.c_str(), "body_max_size:", strlen("body_max_size:")))
		return (fill_body_max_size(file.substr(14), location) + 14);
	if (!strncmp(file.c_str(), "proxy_pass:", strlen("proxy_pass:")))
		return (fill_proxy_pass(file.substr(11), location) + 11);
	if (!strncmp(file.c_str(), "cookie:", strlen("cookie:")))
		return (fill_cookie(file.substr(7), location) + 7);
	if (!strncmp(file.c_str(), "upload_root:", strlen("upload_root:")))
		return (fill_upload_root(file.substr(12), location) + 12);
	return (1);
}

int fill_location(const string &file, ServerBlock &server)
{
	int	i = 0;

	LocationBlock &location = server.addLocation();
	bool exact = false;
	if (file[i] == '=')
	{
		exact = true;
		i++;
	}
	location.setPath(file.substr(i, file.find('{') - i), exact);
	while (file[i] != '{')
		i++;
	i++;
	while (file[i] != '}')
		i += found_location_data(&file[i], location);
	return (i);
}

int found_data(string const &file, ServerBlock &server)
{
	if (!strncmp(file.c_str(), "listen:", strlen("listen:")))
		return (fill_port(file.substr(7), server) + 7);
	if (!strncmp(file.c_str(), "host_name:", strlen("host_name:")))
		return (fill_host_name(file.substr(10), server) + 10);
	if (!strncmp(file.c_str(), "root:", strlen("root:")))
		return (fill_root(file.substr(5), server) + 5);
	if (!strncmp(file.c_str(), "index_file:", strlen("index_file:")))
		return (fill_index_file(file.substr(11), server) + 11);
	if (!strncmp(file.c_str(), "error_pages:", strlen("error_pages:")))
		return (fill_error_pages(file.substr(12), server) + 12);
	if (!strncmp(file.c_str(), "location:", strlen("location:")))
		return (fill_location(file.substr(9), server) + 9);
	if (!strncmp(file.c_str(), "body_max_size:", strlen("body_max_size:")))
		return (fill_body_max_size(file.substr(14), server) + 14);
	if (!strncmp(file.c_str(), "cookie:", strlen("cookie:")))
		return (fill_cookie(file.substr(7), server) + 7);
	if (!strncmp(file.c_str(), "upload_root:", strlen("upload_root:")))
		return (fill_upload_root(file.substr(12), server) + 12);
	return (1);
}

bool	delete_for_fill(char c)
{
	static int simple_quotes = 0;
	static int double_quotes = 0;

	if (c == '\'')
	{
		if (!simple_quotes)
			simple_quotes++;
		else
			simple_quotes--;
	}
	if (c == '"')
	{
		if (!double_quotes)
			double_quotes++;
		else
			double_quotes--;
	}
	if (!simple_quotes && !double_quotes)
		return (c == '\n');
	else
		return (false);
}

void    fill_data(string file, Server *server)
{
	int i = 0;
	int start = 0;

	file.erase(std::remove_if(file.begin(), file.end(), delete_for_fill), file.end());
	while (file[i])
	{
		int index = file.find("server{", start);
		if (index == string::npos)
			return ;
		ServerBlock &block = server->addBlock();
		index += 7;
		start = index;
		while (file[index] && file[index] != '}')
			index += found_data(file.substr(index), block);
	}






	// server->getServerBlock(0).setHostName("localhost");
	// server->getServerBlock(0).setIndex("index.html");

	// server->getServerBlock(0).setSessionCookie("sessionId");

	// // server->getServerBlock(1).set("3000", false);
	// // server->getServerBlock(1).setHostName("virtual.org");
	// // server->getServerBlock(1).setIndex("index.html");
	// // server->getServerBlock(1).setRoot("www");


	// server->getLocationBlock(0, 0).setPath("/");
	// server->getLocationBlock(0, 0).setAutoIndex(true);
	// server->getLocationBlock(0, 0).setHandlers(getMethod, postMethod, NULL);
	// server->getLocationBlock(0, 0).addCgiExtension(".sh", "/bin/sh");
	// server->getLocationBlock(0, 0).addCgiExtension(".py", "/usr/bin/python3");
	// // server->getLocationBlock(0, 0).setHeader("Set-Cookie", "cookie=123; Path=/folder");

	// server->getLocationBlock(0, 1).setPath("////folder////");
	// server->getLocationBlock(0, 1).setHandlers(getMethod, postMethod, NULL);
	// server->getLocationBlock(0, 1).setAutoIndex(true);
	// server->getLocationBlock(0, 1).addHeader("Set-Cookie", "cookie=123; Path=/folder");
	// server->getLocationBlock(0, 1).addErrorPage(404, "404.html");
	// // server->getLocationBlock(0, 1).setRedirection(303, "/");

	// server->getLocationBlock(0, 2).setPath("/upload", false);
	// server->getLocationBlock(0, 2).setHandlers(getMethod, postMethod, deleteMethod);
	// server->getLocationBlock(0, 2).setUploadRoot("www/upload");
	// server->getLocationBlock(0, 2).setAutoIndex(true);
	// server->getLocationBlock(0, 2).addCgiExtension(".py", "/usr/bin/python3");
	// server->getLocationBlock(0, 2).setMaxBodySize(2000000000);

	// server->getLocationBlock(0, 3).setPath("/download", false);
	// server->getLocationBlock(0, 3).addHeader("Content-Disposition", "attachement");
	// server->getLocationBlock(0, 3).setHandlers(getMethod, NULL, NULL);

	// server->getLocationBlock(0, 4).setPath("/folder/dir", false);
	// server->getLocationBlock(0, 4).setHandlers(getMethod, NULL, NULL);
}