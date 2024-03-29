// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "TileLoader.h"
#include "query/TileIndexWalker.h"
#include "gol/tiles/TTile.h"

TileLoader::TileLoader(FeatureStore *store) :
	store_(store),
#ifdef _DEBUG
	threadPool_(1, 0)
#else
	threadPool_(std::thread::hardware_concurrency(), 0)
#endif
{
}


void TileLoader::load()
{
	TileIndexWalker tiw(store_->tileIndex(), store_->zoomLevels(), Box::ofWorld(), nullptr);
	while (tiw.next())
	{
		TileLoaderTask task(store_, tiw.currentTile(), tiw.currentTip());
		threadPool_.post(task);
	}
	threadPool_.awaitCompletion();
}


void TileLoaderTask::operator()()
{
	pointer pTile = store_->fetchTile(tip_);
	// uint32_t size = pTile.getInt() & 0x3fff'ffff;
	// uint8_t* pLoadedTile = new uint8_t[size];

	TTile tile(tile_);
	// store_->prefetchBlob(pTile);
	tile.readTile(pTile);
	/*
	memcpy(pLoadedTile, pTile, size);
	delete[] pLoadedTile;
	*/
}

