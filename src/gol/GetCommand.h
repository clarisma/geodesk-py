// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include "GolCommand.h"
#include <vector>

class GetCommand : CliCommand
{
public:
	GetCommand();

	int run(char* argv[]);

private:
	void setParam(int number, std::string_view value) override;
	void setOption(std::string_view name, std::string_view value) override;

	std::string_view golName_;
	std::vector<std::string_view> tilesetNames_;
	std::string golPath_;
};