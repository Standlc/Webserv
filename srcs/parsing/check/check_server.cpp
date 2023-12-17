#include "../parsing.hpp"

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

int check_host_name(const string &content)
{
	int i = 11;
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

int check_error(string &file)
{
	int index_check = 0;
	error_parsing = 0;
	required["server"] = 0;

	check_brackets(file);
	check_quotes();
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