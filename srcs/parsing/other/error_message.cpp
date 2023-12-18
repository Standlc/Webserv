#include "../parsing.hpp"

int	line_tracker(const string &error, string initializer)
{
	static std::string file = "";

    if (file.empty())
	{
        file = initializer;
	}
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
		if (file[index])
			return (line);
	}
	return (NOLINE);
}

int    error_message(int row, string message, string example)
{
	if (row != NOLINE)
		cerr << "\033[1;34m" << "> ";
	else
		cerr << "\033[0;31m" << "> ";
	if (row != NOLINE)
		cerr << "line " << row << ": ";
	cerr << "\033[0;31m" << message << "\033[0m";
	if (example != NOEX)
		cerr << "\n\033[0;0mExample: " << example;
	cerr << '\n';
	error_parsing = ERR;
	return (0);
}