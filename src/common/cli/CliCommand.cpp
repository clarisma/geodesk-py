// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "CliCommand.h"
#include "CliApplication.h"
#include <stdio.h>

int CliCommand::run(char* argv[])
{
	char* arg;
	int paramCount = 0;
	for (int argCount = 1;;argCount++)
	{
		arg = argv[argCount];
		if (!arg) break;
		if (*arg == '-')
		{
			char* name = arg + ((arg[1] == '-') ? 2 : 1);
			char* value = name;
			while (*value)
			{
				if (*value == '=')
				{
					*value++ = 0;
					break;
				}
				value++;
			}
			if (!setOption(name, value))
			{
				// TODO
				unknownOption(name);
				return 1;
			}
		}
		else
		{
			if (!setParam(paramCount++, arg))
			{
				extraParam(arg);
			}
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


void CliCommand::unknownOption(const char* str)
{
	ConsoleWriter& out = CliApplication::get()->out();
	out.color(196);
	out.writeConstString("Unknown option: ");
	out.writeString(str);
	out.normal();
	out.writeByte('\n');
	out.flush();
}

void CliCommand::extraParam(const char* str)
{
	printf("TODO: Extra param\n");
}
