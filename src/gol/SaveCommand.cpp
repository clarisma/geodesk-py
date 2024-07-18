// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "SaveCommand.h"
#include "gol/load/TileLoader.h"
#include <feature/FeatureStore.h>

SaveCommand::SaveCommand()
{

}

/*
void GetCommand::setParam(int number, std::string_view value)
{
	if (number == 0) return;
	if (number == 1)
	{
		golName_ = value;
	}
	else
	{
		tilesetNames_.push_back(value);
	}
}

void GetCommand::setOption(std::string_view name, std::string_view value)
{

}

*/

int GetCommand::run(char* argv[])
{
	CliCommand::run(argv);
	
	// int threads = std::thread::hardware_concurrency();
	
	FeatureStore store;
	store.open(GolCommand::golPath(golName_).c_str());

	TileLoader loader(&store);
	loader.load();
	return 0;
}
