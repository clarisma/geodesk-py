// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <common/util/ThreadPool.h>
#include "feature/FeatureStore.h"
#include "geom/Tile.h"

class TileLoaderTask
{
public:
	TileLoaderTask(FeatureStore* store, Tile tile, Tip tip) :
		store_(store), tile_(tile), tip_(tip)
	{
	}

	TileLoaderTask() : tip_(0) {} // TODO: Never used, exists only to satisfy compiler

	void operator()();

private:
	FeatureStore* store_;
	Tile tile_;
	Tip tip_;
};


class TileWriterTask
{

};

class TileLoader
{
public:
	TileLoader(FeatureStore* store);
	void load();

private:
	FeatureStore* store_;
	ThreadPool<TileLoaderTask> threadPool_;
};
