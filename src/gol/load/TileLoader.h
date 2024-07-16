// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <common/thread/TaskEngine.h>
// #include "feature/FeatureStore.h"
#include "geom/Tile.h"

class TileLoader;
class FeatureStore;


class TileLoaderTask
{
public:
	TileLoaderTask() {} // TODO: only to satisfy compiler
	TileLoaderTask(Tile tile, int tip) : tile_(tile), tip_(tip) {}

	Tile tile() const { return tile_; }
	int tip() const { return tip_; }

private:
	Tile tile_;
	int tip_;
};


class TileLoaderContext
{
public:
	TileLoaderContext(TileLoader* loader) : loader_(loader) {}
	void processTask(TileLoaderTask& task);
	void afterTasks() {}
	void harvestResults() {}

private:
	TileLoader* loader_;
};

class TileLoaderOutputTask
{
public:
	TileLoaderOutputTask() {} // TODO: only to satisfy compiler
	TileLoaderOutputTask(int tip, std::unique_ptr<uint8_t> data, int32_t size) :
		tip_(tip),
		data_(std::move(data)),
		size_(size)
	{}

	int32_t size() const { return size_; }

private:
	std::unique_ptr<uint8_t> data_;
	int32_t size_;
	int tip_;
};


class TileLoader : public TaskEngine<TileLoader, TileLoaderContext, TileLoaderTask, TileLoaderOutputTask>
{
public:
	TileLoader(FeatureStore* store);

	void load();
	void processTask(TileLoaderOutputTask& task);
	int64_t totalBytesWritten() const { return totalBytesWritten_; }

private:
	FeatureStore* store_;
	double workPerTile_;
	double workCompleted_;
	int64_t totalBytesWritten_;

	friend class TileLoaderContext;
};


