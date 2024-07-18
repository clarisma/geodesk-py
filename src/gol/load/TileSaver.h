// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <common/thread/TaskEngine.h>
#include "geom/Tile.h"
#include "gol/tiles/TesWriter.h"

class TileSaver;
class FeatureStore;


class TileSaverTask
{
public:
	TileSaverTask() {} // TODO: only to satisfy compiler
	TileSaverTask(Tile tile, int tip) : tile_(tile), tip_(tip) {}

	Tile tile() const { return tile_; }
	int tip() const { return tip_; }

private:
	Tile tile_;
	int tip_;
};


class TileSaverContext
{
public:
	TileSaverContext(TileSaver* saver) : saver_(saver) {}
	void processTask(TileSaverTask& task);
	void afterTasks() {}
	void harvestResults() {}

private:
	TileSaver* saver_;
	TesWriter writer_;
};

class TileSaverOutputTask
{
public:
	TileSaverOutputTask() {} // TODO: only to satisfy compiler
	TileSaverOutputTask(int tip, ByteBlock data) :
		tip_(tip),
		data_(std::move(data))
	{}

	const uint8_t* data() const { return data_.data(); }
	int32_t size() const { return data_.size(); }

private:
	ByteBlock data_;
	int tip_;
};


class TileSaver : public TaskEngine<TileSaver, TileSaverContext, TileSaverTask, TileSaverOutputTask>
{
public:
	TileSaver(FeatureStore* store);

	void save(const char* fileName, std::vector<std::pair<Tile,int>> tiles);
	void processTask(TileSaverOutputTask& task);
	int64_t totalBytesWritten() const { return totalBytesWritten_; }

private:
	FeatureStore* store_;
	File out_;
	double workPerTile_;
	double workCompleted_;
	int64_t totalBytesWritten_;

	friend class TileSaverContext;
};


