#include "../parsing.hpp"

int	check_no_backspace(const string &content)
{
	for (int i = 0; content[i] && content[i] != ';'; i++)
		if (content[i] == '\n')
			return (0);
	return (1);
}

int check_quotes()
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
