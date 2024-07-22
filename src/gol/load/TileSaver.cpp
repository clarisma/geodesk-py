// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "TileSaver.h"
#include <zlib.h>
#include "query/TileIndexWalker.h"
#include "tile/model/TileModel.h"
#include "tile/model/TileReader.h"


TileSaver::TileSaver(FeatureStore* store, int threadCount) :
	TaskEngine(threadCount),
	store_(store),
	workCompleted_(0),
	workPerTile_(0),
	totalBytesWritten_(0)
{
}

void TileSaver::save(const char* fileName, std::vector<std::pair<Tile, int>>& tiles)
{
	workPerTile_ = 100.0 / tiles.size();
	workCompleted_ = 0;

	out_.open(fileName, File::CREATE | File::WRITE | File::REPLACE_EXISTING);
	Console::get()->start("Saving...");
	start();

	for(const auto& tile : tiles)
	{
		postWork(TileSaverTask(tile.first, tile.second));
	}
	end();
	// printf("Total bytes written: %lld\n", totalBytesWritten_);
}

void TileSaverContext::processTask(TileSaverTask& task)
{
	FeatureStore* store = saver_->store_;
	pointer pTile = store->fetchTile(task.tip());
	TileModel tile(task.tile());
	TileReader reader(tile);
	// store->prefetchBlob(pTile);
	reader.readTile(pTile.asBytePointer());

	DynamicBuffer buf(1024 * 1024);
	TesWriter writer(tile, &buf);
	writer.write();

	ByteBlock data = buf.takeBytes();
	std::unique_ptr<uint8_t[]> compressed = std::make_unique<uint8_t[]>(
		compressBound(data.size()));
	uLongf compressedSize;
	int result = compress(compressed.get(), &compressedSize, 
		data.data(), data.size());
	if (result != Z_OK)
	{
		// TODO: error
	
	}

	saver_->postOutput(TileSaverOutputTask(task.tip(),
		ByteBlock(std::move(compressed), compressedSize)));
}


void TileSaver::processTask(TileSaverOutputTask& task)
{
	out_.write(task.data(), task.size());
	workCompleted_ += workPerTile_;
	Console::get()->setProgress(static_cast<int>(workCompleted_));
	totalBytesWritten_ += task.size();
}
