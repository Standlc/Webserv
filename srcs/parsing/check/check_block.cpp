#include "../parsing.hpp"

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
		if (number_error >= 600 || number_error < 300)
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
	if (number < 0)
		return (error_message(line_tracker(content.substr(i)), MBODYSIZE, NOEX));
	while (content[i] >= '0' && content[i] <= '9')
		i++;
	if (!(content[i] == ';' || (!strncmp(&content[i], "MB;", strlen("MB;"))
		|| !strncmp(&content[i], "KB;", strlen("KB;")))))
		return (error_message(line_tracker(content.substr(i)), MBODYSIZE, NOEX));
	while (content[i] != ';')
		i++;
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