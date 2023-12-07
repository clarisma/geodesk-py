#pragma once

#include <common/util/ThreadPool.h>
#include "feature/FeatureStore.h"
#include "geom/Tile.h"

#include <fstream>

class TileCompiler;

class TileCompilerTask
{
public:
	TileCompilerTask(TileCompiler* compiler, Tile tile, Tip tip) :
		compiler_(compiler), tile_(tile), tip_(tip)
	{
	}

	TileCompilerTask() : tip_(0) {} // TODO: Never used, exists only to satisfy compiler

	void operator()();

private:
	TileCompiler* compiler_;
	Tile tile_;
	Tip tip_;
};


class TileWriterTask
{
public:
	TileWriterTask(TileCompiler* compiler, Tip tip, const uint8_t* data, uint32_t size) :
		compiler_(compiler), tip_(tip), data_(data), size_(size)
	{
	}

	TileWriterTask() : tip_(0) {} // TODO: Never used, exists only to satisfy compiler

	void operator()();

private:
	TileCompiler* compiler_;
	const uint8_t* data_;
	uint32_t size_;
	Tip tip_;
};

class TileCompiler
{
public:
	TileCompiler(FeatureStore* store);
	void compile();

private:
	FeatureStore* store_;
	ThreadPool<TileCompilerTask> workers_;
	ThreadPool<TileWriterTask> writer_;
	std::ofstream outFile_;

	friend class TileCompilerTask;
	friend class TileWriterTask;
};
