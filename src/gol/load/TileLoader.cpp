// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "TileLoader.h"
#include "query/TileIndexWalker.h"
#include "gol/tiles/IndexSettings.h"
#include "gol/tiles/Layout.h"
#include "gol/tiles/TTile.h"


TileLoader::TileLoader(FeatureStore* store) :
	TaskEngine(1 /* std::thread::hardware_concurrency() */),
	store_(store),
	workCompleted_(0),
	workPerTile_(0),
	totalBytesWritten_(0)
{
}

void TileLoader::load()
{
	// std::vector<int> tips;
	int tileCount = 0;
	TileIndexWalker tiw(store_->tileIndex(), store_->zoomLevels(), Box::ofWorld(), nullptr);
	while (tiw.next())
	{
		tileCount++;
	}

	workPerTile_ = 100.0 / tileCount;
	workCompleted_ = 0;

	Console::get()->start("Loading...");
	start();

	// TODO
	TileIndexWalker tiw2(store_->tileIndex(), store_->zoomLevels(), Box::ofWorld(), nullptr);
	while (tiw2.next())
	{
		postWork(TileLoaderTask(tiw2.currentTile(), tiw2.currentTip()));
	}
	end();
	printf("Total bytes written: %lld\n", totalBytesWritten_);
}

void TileLoaderContext::processTask(TileLoaderTask& task)
{
	FeatureStore* store = loader_->store_;
	pointer pTile = store->fetchTile(task.tip());
	// uint32_t size = pTile.getInt() & 0x3fff'ffff;
	// uint8_t* pLoadedTile = new uint8_t[size];

	TTile tile(task.tile());
	// store->prefetchBlob(pTile);
	tile.readTile(pTile);

	IndexSettings indexSettings(store, 8, 8, 300); // TODO
	Indexer indexer(tile, indexSettings);
	indexer.addFeatures(tile.features());
	indexer.build();

	Layout layout(tile);
	indexer.place(layout);
	layout.flush();
	layout.placeBodies();
	uint8_t* newTileData = tile.write(layout);

	loader_->postOutput(TileLoaderOutputTask(task.tip(), 
		std::move(std::unique_ptr<uint8_t>(newTileData)), layout.size()));
}


void TileLoader::processTask(TileLoaderOutputTask& task)
{
	workCompleted_ += workPerTile_;
	Console::get()->setProgress(static_cast<int>(workCompleted_));
	totalBytesWritten_ += task.size();
}
