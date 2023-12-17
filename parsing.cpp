#include "parsing.hpp"

int isDebug;

map<string, int>	required;
int					error_parsing;

int simple_quotes = 0;
int double_quotes = 0;

int	check_no_backspace(const string &content)
{
	for (int i = 0; content[i] && content[i] != ';'; i++)
		if (content[i] == '\n')
			return (0);
	return (1);
}

int	line_tracker(const string &error, string initializer = "")
{
	static std::string file = "";

    if (file.empty())
        file = initializer;
	if (error != "")
	{
		int sub_string = file.find(error);
		int	line = 1;
		int	index = 0;
		while (file[index] && index != sub_string)
		{
			if (file[index] == '\n')
				line++;
			index++;
		}
		if (index != string::npos)
			return (line);
	}
	return (NOLINE);
}

int    error_message(int row, string message, string example)
{
	if (row != NOLINE)
		cerr << "\033[1;34m" << "line " << row << ":\n";
	cerr << "\033[1;31m" << message << "\033[0m";
	if (example != NOEX)
		cerr << "\n\033[0;0mExample: " << example;
	cerr << '\n';
	error_parsing = ERR;
	return (0);
}

int endline(string line)
{
	int i = 0;

	while (line[i] && line[i] != ';')
		i++;
	if (!line[i])
		return (i - 1);
	return (i);
}

int next_semicolon(string line)
{
	int i = 0;

	while (line[i] && line[i] != ';')
		i++;
	return (i);
}

int next_coma(string line)
{
	int i = 0;

	while (line[i] && line[i] != ',' && line[i] != ';')
		i++;
	if (!line[i])
		return (i - 1);
	return (i);
}

int next_bracket(string line)
{
	int i = 0;

	while (line[i] && line[i] != '{')
		i++;
	if (!line[i])
		return (i - 1);
	return (i);
}

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

int check_error_page(const string &content)
{
	int i = 12;
	while (content[i] == '\n')
		i++;
	if (content[i] != '{')
		return (error_message(line_tracker(content.substr(i)), MBRACK, NOEX));
	i++;
	while (content[i] != '}')
	{
		if (content[i] == '\n')
		{
			i++;
			continue ;
		}
		if (!check_no_backspace(&content[i]))
			return (error_message(line_tracker(content.substr(i)), MERRPAGE, EXERRPAGE));
		int number_error = atoi(&content[i]);
		if (number_error <= 0)
			return (error_message(line_tracker(content.substr(i)), MERRPAGESYNT, EXERRPAGE));
		if (number_error >= 600 || number_error < 100)
			return (error_message(line_tracker(content.substr(i)), MERRNUM, EXERRPAGE));
		i += 3;
		if (content[i] != ':')
			return (error_message(line_tracker(content.substr(i)), MERRPAGE, EXERRPAGE));
		string file_page = content.substr(i + 1, endline(&content[i + 1]));
		if (file_page == "")
			return (error_message(line_tracker(content.substr(i)), MERRPAGE, EXERRPAGE));
		i += endline(&content[i]);
		i++;
	}
	return (i + 1);
}

int check_listen(const string &content)
{
	int i = 7;

	required["listen"] = 1;
	if (!check_no_backspace(content))
		return (error_message(line_tracker(content.substr(i)), MPORT, EXPORT));
	if (!(content[i] >= '0' && content[i] <= '9'))
		return (error_message(line_tracker(content.substr(i)), MPORT, EXPORT));
	while (content[i] >= '0' && content[i] <= '9')
		i++;
	if (content[i] != '.' && content[i] != ';')
		return (error_message(line_tracker(content.substr(i)), MPORT, EXPORT));
	if (content[i] == '.')
	{
		i = 7;
		int count_bytes = 0;
		int	number;
		while (content[i] != ':' && (content[i] == '.' || i == 7))
		{
			if (content[i] == '.')
				i++;
			number = atoi(content.substr(i, content.find('.', i) - i).c_str());
			if (!(number >= 0 && number <= 255))
				return (error_message(line_tracker(content.substr(i)), MPORT, EXPORT));
			count_bytes++;
			i = content.find_first_not_of("0123456789", i);
		}
		if (content[i] != ':' || count_bytes != 4)
			return (error_message(line_tracker(content.substr(i)), MPORT, EXPORT));
		i++;
	}
	else
		i = 7;
	int port = atoi(&content[i]);
	if (port <= 0 || port > 65535)
		return (error_message(line_tracker(content.substr(i)), MPORT, EXPORT));
	while (content[i] >= '0' && content[i] <= '9')
		i++;
	if (content[i] != ';')
		return (error_message(line_tracker(content.substr(i)), MPORT, EXPORT));
	return (i + 1);
}

int check_root(const string &content)
{
	int i = 5;

	required["root"] = 1;
	if (!check_no_backspace(content))
		return (error_message(line_tracker(content.substr(i)), MROOT, EXROOT));
	string root = content.substr(i + 1, endline(&content[i]));
	if (root == "")
		return (error_message(line_tracker(content.substr(i)), MROOT, EXROOT));
	i += endline(&content[i]);
	return (i + 1);
}

int check_host_name(const string &content)
{
	int i = 10;
	if (!check_no_backspace(content))
		return (error_message(line_tracker(content.substr(i)), MHOST, EXHOST));
	do
	{
		if (content[i] == ',')
			i++;
		string name = content.substr(i, next_coma(&content[i]));
		if (name == "")
			return (error_message(line_tracker(content.substr(i)), MHOST, EXHOST));
		i += name.size();
	} while (content[i] != ';' && content[i]);
	return (i + 1);
}

int check_index(const string &content)
{
	int i = 6;
	if (!check_no_backspace(content))
		return (error_message(line_tracker(content.substr(i)), MINDEX, EXINDEX));
	string index = content.substr(i, next_coma(&content[i]));
	if (index == "")
		return (error_message(line_tracker(content.substr(i)), MINDEX, EXINDEX));
	i += endline(&content[i]);
	return (i + 1);
}

int check_upload_root(const string &content)
{
	int i = 12;
	if (!check_no_backspace(content))
		return (error_message(line_tracker(content.substr(i)), MUPLOAD, EXUPLOAD));
	string root = content.substr(i, next_coma(&content[i]));
	if (root == "")
		return (error_message(line_tracker(content.substr(i)), MUPLOAD, EXUPLOAD));
	i += endline(&content[i]);
	return (i + 1);
}

int good_extention(const string &content, int i)
{
	if (!check_no_backspace(&content[i]))
		return (error_message(line_tracker(content.substr(i)), MEXT, EXEXT));
	if (content[i] != '.')
		return (error_message(line_tracker(content.substr(i)), MEXT, EXEXT));
	i++;
	if (!(content[i] > 31 && content[i] < 127) || content[i] == ':')
		return (error_message(line_tracker(content.substr(i)), MEXT, EXEXT));
	while (content[i] != ':' && content[i] != ';' && content[i] != '}' && content[i])
		i++;
	if (content[i] != ':')
		return (error_message(line_tracker(content.substr(i)), MEXT, EXEXT));
	i++;
	if (!(content[i] > 31 && content[i] < 127) || content[i] == ';')
		return (error_message(line_tracker(content.substr(i)), MEXT, EXEXT));
	while (content[i] != ';' && content[i] && content[i] != '}')
		i++;
	if (content[i] != ';')
		return (error_message(line_tracker(content.substr(i)), MSYNT, EXEXT));
	return (i + 1);
}

int check_cgi_extensions(const string &content)
{
	int i = 15;

	while (content[i] == '\n')
		i++;
	if (content[i] != '{')
		return (error_message(line_tracker(content.substr(i)), MBRACK, NOEX));
	i++;
	while (content[i] != '}')
	{
		if (content[i] == '\n')
		{
			i++;
			continue ;
		}
		i = good_extention(content, i);
		if (error_parsing == ERR)
			return (1);
	}
	return (i + 1);
}

int check_auto_index(const string &content)
{
	int i = 11;
	while (content[i] != ';' && content[i])
		i++;
	if (!content[i])
		return (error_message(line_tracker(content.substr(i)), MSYNT, NOEX));
	string instruction = content.substr(11, i - 11);
	if (!(instruction == "on" || instruction == "off"))
		return (error_message(line_tracker(content.substr(i)), MAUTIND, EXAUTIND));
	return (i + 1);
}

int good_header(const string &content, int i)
{
	if (!check_no_backspace(&content[i]))
		return (error_message(line_tracker(content.substr(i)), MHEAD, EXHEADER));
	if (!(content[i] > 31 && content[i] < 127) || content[i] == ':')
		return (error_message(line_tracker(content.substr(i)), MHEAD, EXHEADER));
	while (content[i] && content[i] != '}' && content[i] != ';' && content[i] != ':')
		i++;
	if (content[i] != ':')
		return (error_message(line_tracker(content.substr(i)), MHEAD, EXHEADER));
	i++;
	while (content[i] && content[i] != '}' && content[i] != ';' && content[i] != ':')
		i++;
	if (content[i] != ';')
		return (error_message(line_tracker(content.substr(i)), MHEAD, EXHEADER));
	return (i + 1);
}

int check_add_header(const string &content)
{
	int i = 12;

	while (content[i] == '\n')
		i++;
	if (content[i] != '{')
		return (error_message(line_tracker(content.substr(i)), MBRACK, NOEX));
	i++;
	while (content[i] != '}')
	{
		if (content[i] == '\n')
		{
			i++;
			continue ;
		}
		i = good_header(content, i);
		if (error_parsing == ERR)
			return (1);
	}
	return (i + 1);
}

int check_quotes(const string &content)
{
	if (simple_quotes || double_quotes)
		return (error_message(NOLINE, MQUOTES, NOEX));
	return (0);
}

int check_brackets(const string &content)
{
	int i = 0;
	int count_brackets = 0;

	while (content[i])
	{
		if (content[i] == '{')
			count_brackets++;
		if (content[i] == '}')
			count_brackets--;
		if (count_brackets < 0)
			return (error_message(line_tracker(content.substr(i)), MBRACK, NOEX));
		i++;
	}
	if (count_brackets != 0)
		return (error_message(line_tracker(content.substr(i)), MBRACK, NOEX));
	return (0);
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
		else
			return (error_message(line_tracker(content.substr(i)), MMETHOD, NOEX));
		if (content[i] == ',')
			i++;
	}
	return (i + 1);
}

int check_cookie(const string &content)
{
	int i = 7;

	string cookie = content.substr(i, content.find(';') - i);
	int index_cookie = 0;
	while (cookie[index_cookie])
	{
		if (cookie[index_cookie] == ' ' || cookie[index_cookie] == '\t' || cookie[index_cookie] == '\n')
			return (error_message(line_tracker(content.substr(i)), MCOOKIE, EXCOOKIE));
		index_cookie++;
	}
	return (index_cookie + i + 1);
}

int check_body_max_size(const string &content)
{
	int i = 14;

	int number = atoi(&content[i]);
	if (number <= 0)
		return (error_message(line_tracker(content.substr(i)), MBODYSIZE, NOEX));
	while (content[i] >= '0' && content[i] <= '9')
		i++;
	if (!(content[i] == ';' || !strncmp(&content[i], "MB;", strlen("MB;"))))
		return (error_message(line_tracker(content.substr(i)), MBODYSIZE, NOEX));
	while (content[i] != ';')
		i++;
	return (i + 1);
}

int check_redirect(const string &content)
{
	int i = 9;

	if (!check_no_backspace(content))
		return (error_message(line_tracker(content.substr(i)), MREDIRECT, NOEX));
	int number = atoi(&content[i]);
	if (!(number >= 300 && number <= 308))
		return (error_message(line_tracker(content.substr(i)), MREDIRECT, NOEX));
	while (content[i] >= '0' && content[i] <= '9')
		i++;
	if (content[i] != ',')
		return (error_message(line_tracker(content.substr(i)), MSYNT, NOEX));
	i++;
	string root = content.substr(i, next_semicolon(&content[i]));
	if (root == "")
		return (error_message(line_tracker(content.substr(i)), MREDIRECT, NOEX));
	i += root.size();
	return (i + 1);
}

int check_fallback(const string &content)
{
	int i = 9;

	if (!check_no_backspace(content))
		return (error_message(line_tracker(content.substr(i)), MFALLBACK, EXFALLBACK));
	string root = content.substr(i, next_semicolon(&content[i]));
	if (root == "")
		return (error_message(line_tracker(content.substr(i)), MFALLBACK, EXFALLBACK));
	i += root.size();
	return (i + 1);
}

int check_block(const string &content)
{
	if (content[0] == '\n')
		return (1);
	if (!strncmp(content.c_str(), "root:", strlen("root:")))
		return (check_root(content));
	if (!strncmp(content.c_str(), "methods:", strlen("methods:")))
		return (check_methods(content));
	if (!strncmp(content.c_str(), "index:", strlen("index:")))
		return (check_index(content));
	if (!strncmp(content.c_str(), "auto_index:", strlen("auto_index:")))
		return (check_auto_index(content));
	if (!strncmp(content.c_str(), "cgi_extensions:", strlen("cgi_extensions")))
		return (check_cgi_extensions(content));
	if (!strncmp(content.c_str(), "add_headers:", strlen("add_headers:")))
		return (check_add_header(content));
	if (!strncmp(content.c_str(), "body_max_size:", strlen("body_max_size:")))
		return (check_body_max_size(content));
	if (!strncmp(content.c_str(), "redirect:", strlen("redirect:")))
		return (check_redirect(content));
	if (!strncmp(content.c_str(), "fallback:", strlen("fallback:")))
		return (check_fallback(content));
	if (!strncmp(content.c_str(), "proxy_pass:", strlen("proxy_pass:")))
		return (check_proxy_pass(content));
	if (!strncmp(content.c_str(), "error_pages:", strlen("error_pages:")))
		return (check_error_page(content));
	if (!strncmp(content.c_str(), "cookie:", strlen("cookie:")))
		return (check_cookie(content));
	if (!strncmp(content.c_str(), "upload_root:", strlen("upload_root:")))
		return (check_upload_root(content));
	return (error_message(line_tracker(content), MDATA, NOEX));
}

int check_location(const string &content)
{
	int i = 9;

	required["location"] = 1;
	string path = content.substr(i, content.find('{') - i);
	if (path == "")
		return (error_message(line_tracker(content.substr(i)), MLOCFOL, NOEX));
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

int check_server(const string &content)
{
	int i = 6;

	required["server"] = 1;
	while (content[i] == '\n' && content[i])
		i++;
	if (content[i] != '{')
		return (error_message(line_tracker(content.substr(i)), MBRACK, NOEX));
	return (i + 1);
}

int check_content(const string &content)
{
	if (content[0] == '\n' || content[0] == '}')
		return (1);
	if (!strncmp(content.c_str(), "server", strlen("server")))
		return (check_server(content));
	if (!strncmp(content.c_str(), "listen:", strlen("listen:")))
		return (check_listen(content));
	if (!strncmp(content.c_str(), "root:", strlen("root:")))
		return (check_root(content));
	if (!strncmp(content.c_str(), "host_name:", strlen("host_name:")))
		return (check_host_name(content));
	if (!strncmp(content.c_str(), "index:", strlen("index:")))
		return (check_index(content));
	if (!strncmp(content.c_str(), "error_pages:", strlen("error_pages:")))
		return (check_error_page(content));
	if (!strncmp(content.c_str(), "location:", strlen("location:")))
		return (check_location(content));
	if (!strncmp(content.c_str(), "auto_index:", strlen("auto_index:")))
		return (check_auto_index(content));
	if (!strncmp(content.c_str(), "cgi_extensions:", strlen("cgi_extensions")))
		return (check_cgi_extensions(content));
	if (!strncmp(content.c_str(), "body_max_size:", strlen("body_max_size:")))
		return (check_body_max_size(content));
	if (!strncmp(content.c_str(), "add_headers:", strlen("add_headers:")))
		return (check_add_header(content));
	if (!strncmp(content.c_str(), "cookie:", strlen("cookie:")))
		return (check_cookie(content));
	if (!strncmp(content.c_str(), "upload_root:", strlen("upload_root:")))
		return (check_upload_root(content));
	if (!strncmp(content.c_str(), "fallback:", strlen("fallback:")))
		return (check_fallback(content));
	return (error_message(line_tracker(content), MDATA, NOEX));
}

void	set_required()
{
	required["listen"] = 0;
	required["root"] = 0;
	required["location"] = 0;
}

int	check_required()
{
	if (required["listen"] == 0)
		return (error_message(NOLINE, MNOLISTEN, NOEX));
	if (required["root"] == 0)
		return (error_message(NOLINE, MNOROOT, NOEX));
	return (1);
}

int check_error(string &file)
{
	int index_check = 0;
	error_parsing = 0;
	required["server"] = 0;

	check_brackets(file);
	check_quotes(file);
	if (error_parsing == ERR)
		return (ERR);
	while (file[index_check])
	{
		if (!strncmp(file.substr(index_check).c_str(), "debug=on", strlen("debug=on")))
			isDebug = 1;
		if (!strncmp(file.substr(index_check).c_str(), "server", strlen("server")))
		{
			set_required();
			while (file[index_check] != '}' && file[index_check])
			{
				if (error_parsing == ERR)
					return (ERR);
				index_check += check_content(file.substr(index_check));
			}
			if (!check_required())
				return (ERR);
			if (required["location"] == 0)
				file = file.substr(0, index_check - 1) + "location:/{methods:GET;}" + file.substr(index_check);
		}
		if (file[index_check])
			index_check++;
	}
	if (required["server"] == 0)
		return (error_message(NOLINE, MNOSERV, NOEX));
	return (0);
}

void delete_comment(string &file)
{
	int i = 0;

	while (file.size() && file[i])
	{
		if (file[i] == '#')
		{
			int backspace = file.find('\n', i);
			file.erase(i, backspace - i);
			while (i > 0 && file[i] != '\n')
				i--;
		}
		i++;
	}
}

bool delete_for_parse(char c)
{
	if (c == '\'' && !double_quotes)
	{
		if (!simple_quotes)
			simple_quotes++;
		else
			simple_quotes--;
		return (1);
	}
	if (c == '"' && !simple_quotes)
	{
		if (!double_quotes)
			double_quotes++;
		else
			double_quotes--;
		return (1);
	}
	if (!simple_quotes && !double_quotes)
		return (c == ' ' || c == '\t');
	else
		return (false);
}

int parsing(int argc, char **argv, Server *server)
{
	if (argc != 2)
	{
		error_message(NOLINE, NOARG, NOEX);
		return (1);
	}

	ifstream    config_file(argv[1]);

	if (!config_file)
	{
		error_message(NOLINE, NOFILE, NOEX);
		return (1);
	}

	string file((istreambuf_iterator<char>(config_file)), istreambuf_iterator<char>());

	config_file.close();

	delete_comment(file);

	simple_quotes = 0;
	double_quotes = 0;
	file.erase(std::remove_if(file.begin(), file.end(), delete_for_parse), file.end());
	line_tracker("", file);

	if (check_error(file) == ERR)
		return (ERR);

	fill_data(file, server);

	return (0);
}