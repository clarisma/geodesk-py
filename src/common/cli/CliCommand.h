// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <string_view>

class CliCommand
{
public:
	static const char* getCommand(char* argv[]);
	int run(char* argv[]);
	virtual bool setParam(int number, const char* value) { return false; };
	virtual bool setOption(const char* name, const char* value) { return false; };

private:
	void unknownOption(const char* str);
	void extraParam(const char* str);
};
