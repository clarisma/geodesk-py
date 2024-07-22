// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "GetCommand.h"
#include "GolCommand.h"
#include "gol/load/TileLoader.h"
#include <feature/FeatureStore.h>

GetCommand::GetCommand()
{

}

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

int GetCommand::run(char* argv[])
{
	CliCommand::run(argv);
	
	int threads = std::thread::hardware_concurrency();
#ifdef _DEBUG
	threads = 1;
#endif
	
	FeatureStore store;
	store.open(GolCommand::golPath(golName_).c_str());

	TileLoader loader(&store, threads);
	loader.load();
	return 0;
}
