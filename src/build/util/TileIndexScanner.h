// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include "feature/ZoomLevels.h"
#include "geom/Tile.h"

// void branchTile(uint32_t parentTip, Tile tile, uint32_t tip);
// void leafTile(uint32_t parentTip, Tile tile, uint32_t tip);
// void omittedTile(uint32_t parentTip, Tile tile);

template <typename Derived>
class TileIndexScanner
{
public:
	TileIndexScanner(const uint32_t* index, ZoomLevels levels) :
		index_(index),
		levels_(levels)
	{
	}

	void scan()
	{
		uint32_t steps = 0;
		int bitPos = 0;
		ZoomLevels::Iterator iter = levels_.iter();
		for (;;)
		{
			int zoom = iter.next();
			if (zoom < 0) break;
			steps |= (levels_.skippedAfterLevel(zoom) + 1) << bitPos;
			bitPos += 2;
		}
		scanBranch(0, Tile::fromColumnRowZoom(0,0,0), 1, steps);
	}

	const uint32_t* index() const { return index_; }

private:
	Derived* self() { return reinterpret_cast<Derived*>(this); }

	void scanBranch(uint32_t parentTip, Tile tile, uint32_t tip, uint32_t steps)
	{
		self()->branchTile(parentTip, tile, tip);
		uint32_t step = steps & 3;
		int zoom = tile.zoom() + step;
		int top = tile.row() << step;
		int left = tile.column() << step;
		uint64_t childMask;
		uint32_t childTip;
		if(step == 3)
		{ 
			childMask = static_cast<uint64_t>(index_[tip + 1]) |
				(static_cast<uint64_t>(index_[tip + 2]) << 32);
			childTip = tip + 3;
		}
		else
		{
			childMask = index_[tip + 1];
			childTip = tip + 2;
		}
		int extent = 1 << step;
		int right = left + extent - 1;
		int bottom = top + extent - 1;
		for (int row = top; row <= bottom; row++)
		{
			for (int col = left; col <= right; col++)
			{
				Tile childTile = Tile::fromColumnRowZoom(col, row, zoom);
				if (childMask & 1)
				{
					uint32_t childEntry = index_[childTip];
					if ((childEntry & 3) == 1)
					{
						//printf("Scanning child branch...\n");
						scanBranch(tip, childTile, 
							childTip + (static_cast<int32_t>(childEntry) >> 2), 
							steps >> 2);
					}
					else
					{
						self()->leafTile(tip, childTile, childTip);
					}
					childTip++;
				}
				else
				{
					self()->omittedTile(tip, childTile);
				}
				childMask >>= 1;
			}
		}
	}

	const uint32_t* index_;
	const ZoomLevels levels_;
};