// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "CliCommand.h"

int CliCommand::run(char* argv[])
{
	const char* arg;
	int paramCount = 0;
	for (int argCount = 1;;argCount++)
	{
		arg = argv[argCount];
		if (!arg) break;
		if (*arg == '-')
		{
			const char* name = arg + ((arg[1] == '-') ? 2 : 1);
			const char* value = name;
			while (*value && *value != '=') value++;
			setOption(std::string_view(name, value - name),	std::string_view(value));
		}
		else
		{
			setParam(paramCount++, std::string_view(arg));
		}
	}
	return 0;
}


const char* CliCommand::getCommand(char* argv[])
{
	char** pArg = &argv[1];
	while (*pArg)
	{
		if (**pArg != '-') return *pArg;
		pArg++;
	}
	return NULL;
}
