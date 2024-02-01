// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "TileCompiler.h"
#include "query/TileIndexWalker.h"
#include "IndexSettings.h"
#include "Layout.h"
#include "TTile.h"
#include "TIndex.h"
#include "TesWriter.h"
#include "build/Analyzer.h"

#include <thread>
#include <chrono>

TileCompiler::TileCompiler(FeatureStore* store) :
	store_(store),
#ifdef _DEBUG
	workers_(1, 0),
#else
	workers_(std::thread::hardware_concurrency(), 0),
#endif
	writer_(1, 8)
{
}


void TileCompiler::compile()
{
#ifdef _DEBUG
	int threads = 1;
#else
	int threads = std::thread::hardware_concurrency();
#endif
	Analyzer reader(threads);
	// reader.read("e:\\geodesk\\mapdata\\de-2021-01-29.osm.pbf");
	reader.read("e:\\geodesk\\mapdata\\planet-2023-10-07.osm.pbf");
	return;

	outFile_= std::ofstream("e:\\geodesk\\exports\\planet-tes.bin", std::ios::binary);
	TileIndexWalker tiw(store_->tileIndex(), store_->zoomLevels(), Box::ofWorld(), nullptr);
	while (tiw.next())
	{
		TileCompilerTask task(this, tiw.currentTile(), tiw.currentTip());
		workers_.post(task);
	}
	workers_.awaitCompletion();
	writer_.awaitCompletion();
	workers_.shutdown();
	writer_.shutdown();
	outFile_.close();
}


void TileCompilerTask::operator()()
{
	FeatureStore* store = compiler_->store_;
	pointer pTile = store->fetchTile(tip_);
	// uint32_t size = pTile.getInt() & 0x3fff'ffff;
	// uint8_t* pLoadedTile = new uint8_t[size];

	TTile tile(tile_);
	// store->prefetchBlob(pTile);
	tile.readTile(pTile);

	DynamicBuffer buf(128 * 1024);
	TesWriter writer(tile, &buf);
	writer.write();
	

	size_t size = buf.length();
	compiler_->writer_.post(TileWriterTask(compiler_, tip_, 
		reinterpret_cast<uint8_t*>(buf.take()), size));

	/*
	IndexSettings indexSettings(store, 8, 8, 300); // TODO
	Indexer indexer(tile, indexSettings);
	indexer.addFeatures(tile.features());
	indexer.build();

	Layout layout(tile);
	indexer.place(layout);
	layout.flush();
	layout.placeBodies();
	uint8_t* newTileData = tile.write(layout);
	delete newTileData;
	*/

	// compiler_->writer_.post(TileWriterTask(compiler_, tip_, newTileData, layout.size()));
	/*
	memcpy(pLoadedTile, pTile, size);
	delete[] pLoadedTile;
	*/
}


void TileWriterTask::operator()()
{
	compiler_->outFile_.write(reinterpret_cast<const char*>(data_), size_);

	//uint8_t* copy = new uint8_t[size_];
	// memcpy(copy, data_, size_);
	delete[] data_;
	// std::this_thread::sleep_for(std::chrono::microseconds(size_ / 500));
	// delete[] copy;
}
