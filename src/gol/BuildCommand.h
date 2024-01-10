// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include "GolCommand.h"

class BuildCommand : CliCommand
{
public:
	BuildCommand();

	void run(char* argv[]);

private:
	void setParam(int number, std::string_view value) override;
	void setOption(std::string_view name, std::string_view value) override;

	std::string golPath_;
	std::string sourcePath_;
};
