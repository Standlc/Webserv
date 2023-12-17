#include "../parsing.hpp"

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