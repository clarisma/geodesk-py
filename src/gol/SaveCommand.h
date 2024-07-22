// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include "GolCommand.h"
#include <vector>

class SaveCommand : GolCommand
{
public:
	SaveCommand();

	int run(char* argv[]);

private:
	void setParam(int number, std::string_view value) override;
	//void setParam(int number, std::string_view value) override;
	//void setOption(std::string_view name, std::string_view value) override;

	std::string tesPath_;
};