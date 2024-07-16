// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <common/cli/Console.h>

class GolTool
{
public:
	GolTool();
	int run(char* argv[]);

private:
	Console console_;
};
