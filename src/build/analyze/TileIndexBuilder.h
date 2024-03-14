// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <common/alloc/Arena.h>
#include "build/util/BuildSettings.h"
#include "geom/Tile.h"


class TileIndexBuilder
{
public:
	TileIndexBuilder(const BuildSettings& settings);
	void build(std::unique_ptr<const uint32_t[]> nodeCounts);
	int tileCount() const { return tileCount_; }
	std::unique_ptr<const uint32_t[]> takeTileIndex() { return std::move(tileIndex_); }
	std::unique_ptr<const uint32_t[]> takeTileSizeEstimates()
	{ 
		return std::move(tileSizeEstimates_); 
	}

private:
	class STile
	{
	public:
		STile(Tile tile, uint32_t maxChildren, uint64_t nodeCount, STile* next);

		Tile tile() const { return tile_; }
		uint32_t tip() const { return location_ / 4; }
		void setParent(STile* pt) { parent_ = pt; }
		uint64_t nodeCount() const { return totalNodeCount_; }
		uint64_t estimatedTileSize() const { return estimatedTileSize_; }
		STile* next() const { return next_; }
		void setNext(STile* next) { next_ = next; }
		void clearNodeCount() {	totalNodeCount_ = 0; }
		void addNodeCount(const STile* ct) 
		{ 
			totalNodeCount_ += ct->totalNodeCount_;
		}

		void addEstimatedSize(int64_t estimatedSize)
		{
			estimatedTileSize_ += estimatedSize;
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
		STile* next_;
		Tile tile_;
		uint32_t location_;
		uint32_t maxChildren_;
		uint32_t childCount_;
		uint64_t totalNodeCount_;
		uint64_t estimatedTileSize_;
		// uint64_t ownNodeCount_;
		STile* parent_;
		STile* children_[1];

		friend class TileIndexBuilder;
	};

	struct Tier
	{
		STile* firstTile;
		uint32_t tileCount;
		uint8_t level;
		uint8_t skippedLevels;

		uint32_t maxChildCount() const
		{
			return 1 << ((skippedLevels + 1) * 2);
		}

		class Iterator
		{
		public:
			Iterator(STile* first) : next_(first) {}
			STile* next() noexcept
			{
				STile* tile = next_;
				if(tile) next_ = tile->next();
				return tile;
			}

		private:
			STile* next_;
		};

		Iterator iter() const noexcept { return Iterator(firstTile); }

		void addTile(STile* tile)
		{
			tile->setNext(firstTile);
			firstTile = tile;
			tileCount++;
		}

		void clearTiles()
		{
			firstTile = nullptr;
			tileCount = 0;
		}
	};

	void createLeafTiles(const uint32_t* nodeCounts);
	void addParentTiles();
	void trimTiles();
	void linkChildTiles();
	uint32_t layoutIndex();
	STile* createTile(Tile tile, uint32_t maxChildCount, 
		uint64_t nodeCount, STile* next);
	uint32_t sumTileCounts() noexcept;
	void calculateTileSizeEstimates();
	int tileSizeEstimate(int pile, STile* tile);

	static const int MAX_TIERS = 8;
	static const int ESTIMATED_BYTES_PER_NODE = 8;
	static const int ESTIMATED_CHILD_BYTES_PER_PARENT_BYTE = (1 << 14);

	Arena arena_;
	const BuildSettings& settings_;
	Tier tiers_[MAX_TIERS + 1];
		// We need one extra tier in case Zoom 12 is not 
		// part of the Tile Pyramid
	int tierCount_;
	int tileCount_;
	std::unique_ptr<uint32_t[]> tileIndex_;
	std::unique_ptr<uint32_t[]> tileSizeEstimates_;
};
