#include "../parsing.hpp"

bool delete_for_fill(char c) {
    return (c == '\n');
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