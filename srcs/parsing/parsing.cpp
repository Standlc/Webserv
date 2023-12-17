#include "parsing.hpp"

int isDebug;

map<string, int>	required;
int					error_parsing;

int simple_quotes = 0;
int double_quotes = 0;

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

	if (!fill_data(file, server))
	{
		error_message(NOLINE, MSAMESERV, NOEX);
		return (ERR);
	}

	return (0);
}