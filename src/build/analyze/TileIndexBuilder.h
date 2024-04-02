// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <common/alloc/Arena.h>
#include "build/util/BuildSettings.h"
#include "geom/Tile.h"

struct STile;

class TileIndexBuilder
{
public:
	TileIndexBuilder(const BuildSettings& settings);
	void build(std::unique_ptr<const uint32_t[]> nodeCounts);
	int tileCount() const { return tileCount_; }
	ZoomLevels zoomLevels() { return settings_.zoomLevels(); }

	std::unique_ptr<const uint32_t[]> takeTileIndex() { return std::move(tileIndex_); }
	std::unique_ptr<const int[]> takeCellToPile() { return std::move(cellToPile_); }
	std::unique_ptr<const int[]> takeTipToPile() { return std::move(tipToPile_); }
	std::unique_ptr<const Tile[]> takePileToTile() { return std::move(pileToTile_); }
	std::unique_ptr<const uint32_t[]> takeTileSizeEstimates() { return std::move(tileSizeEstimates_); }

	static const int ESTIMATED_BYTES_PER_NODE = 8;
	static const int ESTIMATED_CHILD_BYTES_PER_PARENT_BYTE = (1 << 14);

private:
	struct Tier
	{
		STile* firstTile;
		uint32_t tileCount;
		uint8_t level;
		uint8_t skippedLevels;

		class Iterator
		{
		public:
			Iterator(STile* first) : next_(first) {}
			STile* next() noexcept;
			
		private:
			STile* next_;
		};

		Iterator iter() const noexcept { return Iterator(firstTile); }
			
		uint32_t maxChildCount() const
		{
			return 1 << ((skippedLevels + 1) * 2);
		}

		void addTile(STile* tile);

		void clearTiles()
		{
			firstTile = nullptr;
			tileCount = 0;
		}
	};

	constexpr int cellOf(int col, int row)
	{
		assert(col >= 0 && col < (1 << maxZoom_));
		assert(row >= 0 && row < (1 << maxZoom_));
		return row * (1 << maxZoom_) + col;
	}

	void createLeafTiles(const uint32_t* nodeCounts);
	void addParentTiles();
	void trimTiles();
	void linkChildTiles();
	STile* createTile(Tile tile, int maxChildCount, uint64_t nodeCount, STile* next);
	static void addChildTile(STile* parent, STile* child);
	static int assignTip(STile* tile, int tip) noexcept;
	void buildTile(STile* tile) noexcept;
	uint32_t sumTileCounts() noexcept;
	void fillGrid(Tile tile, int pile);

	static const int MAX_TIERS = 8;

	Arena arena_;
	const BuildSettings& settings_;
	Tier tiers_[MAX_TIERS + 1];
		// We need one extra tier in case Zoom 12 is not 
		// part of the Tile Pyramid
	int tierCount_;
	int tileCount_;
	int pileCount_;
	int maxZoom_;
	std::unique_ptr<uint32_t[]> tileIndex_;
	std::unique_ptr<uint32_t[]> tileSizeEstimates_;
	std::unique_ptr<int[]> cellToPile_;
	std::unique_ptr<int[]> tipToPile_;
	std::unique_ptr<Tile[]> pileToTile_;
};
