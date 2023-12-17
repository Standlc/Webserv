#include "../parsing.hpp"

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