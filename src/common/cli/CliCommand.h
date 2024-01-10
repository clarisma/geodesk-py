// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <string_view>

class CliCommand
{
public:
	void run(char* argv[]);
	virtual void setParam(int number, std::string_view value) {};
	virtual void setOption(std::string_view name, std::string_view value) {};
};
