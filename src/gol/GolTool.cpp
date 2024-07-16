// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "GolTool.h"
#include "GetCommand.h"

GolTool::GolTool()
{

}

int GolTool::run(char* argv[])
{
	const char* cmd = CliCommand::getCommand(argv);
	if (cmd)
	{
		if (strcmp(cmd, "get") == 0)
		{
			return GetCommand().run(argv);
		}
	}
}
