// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "SaveCommand.h"
#include "gol/load/TileSaver.h"
#include <feature/FeatureStore.h>
#include "query/TileIndexWalker.h"

SaveCommand::SaveCommand()
{

}

void SaveCommand::setParam(int number, std::string_view value)
{
	if (number == 2)
	{
		tesPath_ = value;
	}
	GolCommand::setParam(number, value);
}

/*
void GetCommand::setOption(std::string_view name, std::string_view value)
{

}

*/

int SaveCommand::run(char* argv[])
{
	GolCommand::run(argv);

	int threads = std::thread::hardware_concurrency();
#ifdef _DEBUG
	threads = 1;
#endif

	FeatureStore store;
	store.open(golPath_.c_str());

	std::vector<std::pair<Tile, int>> tiles;
	TileIndexWalker tiw(store.tileIndex(), store.zoomLevels(), Box::ofWorld(), nullptr);
	while (tiw.next())
	{
		tiles.push_back({ tiw.currentTile(), tiw.currentTip() });
	}

	TileSaver saver(&store, threads);
	saver.save(tesPath_.c_str(), tiles);
	return 0;
	// int threads = std::thread::hardware_concurrency();
	
	/*
	FeatureStore store;
	store.open(GolCommand::golPath(golName_).c_str());

	TileLoader loader(&store);
	loader.load();
	*/
	return 0;
}
