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
	const uint32_t* build(const uint32_t* nodeCounts);

private:
	class STile
	{
	public:
		STile(Tile tile, uint32_t maxChildren, uint64_t nodeCount) :
			location_(0),
			tile_(tile),
			maxChildren_(maxChildren),
			childCount_(0),
			totalNodeCount_(nodeCount),
			// ownNodeCount_(0),
			parent_(nullptr)
		{
		}

		Tile tile() const { return tile_; }
		uint32_t tip() const { return location_ / 4; }
		void setParent(STile* pt) { parent_ = pt; }
		uint64_t count() const { return totalNodeCount_; }
		void addCount(const STile* ct) 
		{ 
			totalNodeCount_ += ct->totalNodeCount_;
		}

		void addChild(STile* t)
		{
			assert(childCount_ <= maxChildren_);
			children_[childCount_++] = t;
		}

		STile* parent() const { return parent_; }

		const STile* const* children() const noexcept
		{
			return children_;
		}

		bool isLeaf() const noexcept { return childCount_ == 0; }

		uint32_t size() const noexcept
		{
			return ((maxChildren_ > 32) ? 12 : 8) + childCount_ * 4;
		}

		uint32_t layout(uint32_t pos) noexcept;
		void write(uint32_t* p) noexcept;

	private:
		Tile tile_;
		uint32_t location_;
		uint32_t maxChildren_;
		uint32_t childCount_;
		uint64_t totalNodeCount_;
		// uint64_t ownNodeCount_;
		STile* parent_;
		STile* children_[1];
	};

	void createLeafTiles(const uint32_t* nodeCounts);
	void addParentTiles();
	void linkChildTiles();
	uint32_t layoutIndex();
	STile* createTile(Tile tile, uint32_t maxChildCount, uint64_t nodeCount);
	uint32_t maxChildCountOfTier(int tier)
	{
		return 1 << ((tierSkippedLevelCounts_[tier] + 1) * 2);
	}

	static const int MAX_TIERS = 8;

	Arena arena_;
	std::vector<STile*> tiles_;
	STile* root_;
	const BuildSettings& settings_;
	uint32_t tierCount_;
	uint8_t tierLevels_[MAX_TIERS];
	uint8_t tierSkippedLevelCounts_[MAX_TIERS];
};
