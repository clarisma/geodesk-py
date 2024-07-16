#include "GolCommand.h"


GolCommand::GolCommand()
{

}

void GolCommand::setParam(int number, std::string_view value) 
{
	if (number == 0) return;
	if (number == 1)
	{
		golPath_ = golPath(value);
		return;
	}
	// TODO: Superfluous arguments
}

void GolCommand::setOption(std::string_view name, std::string_view value) 
{

}


