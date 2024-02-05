// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <common/alloc/Arena.h>
#include "build/util/BuildSettings.h"
#include "geom/Tile.h"


/*
struct TileSettings
{
	uint32_t maxTiles = 64 * 1024 - 1;
	uint32_t minTileDensity = 25000;
	int leafZoomLevel = 12;
	ZoomLevels zoomLevels;
};
*/

class TileIndexBuilder
{
public:
	TileIndexBuilder(const BuildSettings& settings);
	void build(const uint32_t* nodeCounts);

private:
	void createLeafTiles(const uint32_t* nodeCounts);
	void addParentTiles();

	class STile
	{
	public:
		STile(Tile tile, uint64_t nodeCount) :
			tile_(tile),
			totalNodeCount_(nodeCount),
			parent_(nullptr),
			children_(nullptr)
		{
		}

		Tile tile() const { return tile_; }
		void setParent(STile* pt) { parent_ = pt; }
		uint64_t count() const { return totalNodeCount_; }
		void addCount(const STile* ct) 
		{ 
			totalNodeCount_ += ct->totalNodeCount_;
		}

	private:
		Tile tile_;
		uint64_t totalNodeCount_;
		uint64_t ownNodeCount_;
		STile* parent_;
		STile** children_;
	};

	Arena arena_;
	std::vector<STile*> tiles_;
	const BuildSettings& settings_;
};
