// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cassert>
#include <cstdint>
#include "TileIndexScanner.h"

class TileCatalog
{
public:
	static const int MAX_ZOOM = 12;

	void build(uint32_t tileCount, const uint32_t* index, ZoomLevels levels);

	uint32_t tileCount() const { return tileCount_; }

	Tile tileOfPile(uint32_t pile) const
	{
		return piles_[pile].tile;
	}

	uint32_t pileOfCoordinate(Coordinate c) const
	{
		int col = Tile::columnFromXZ(c.x, MAX_ZOOM);
		int row = Tile::rowFromYZ(c.y, MAX_ZOOM);
		// printf("12/%d/%d\n", col, row);
		return grid_.get()[cellOf(col, row)];
	}

private:
	class Builder : TileIndexScanner<Builder>
	{
	public:
		Builder(const uint32_t* index, ZoomLevels levels) :
			TileIndexScanner(index, levels),
			tileCount_(0)
		{
		}

		void build();
		void tile(uint32_t parentTip, Tile tile, uint32_t tip);
		void omittedTile(uint32_t parentTip, Tile tile);
		const uint32_t* takeGrid() { return grid_.release(); }
		const uint32_t* takeTipToPile() { return tipToPile_.release(); }
		uint32_t tileCount() const { return tileCount_; }

	private:
		std::unique_ptr<uint32_t[]> grid_;
		std::unique_ptr<uint32_t[]> tipToPile_;
		uint32_t tileCount_;
	};

	struct PileEntry
	{
		Tile tile;
		uint32_t parentPile;
	};

	static constexpr int cellOf(int col, int row)
	{
		assert(col >= 0 && col < (1 << MAX_ZOOM));
		assert(row >= 0 && row < (1 << MAX_ZOOM));
		return row * (1 << MAX_ZOOM) + col;
	}

	std::unique_ptr<const uint32_t[]> grid_;
	std::unique_ptr<const uint32_t[]> tipToPile_;

	const PileEntry* piles_;
	/**
	 * A table used to look up the pile number of the tile 
	 * to the east of a given tile
	 */
	const uint32_t* eastPiles_;
	
	/**
	 * A table used to look up the pile number of the tile
	 * to the south of a given tile
	 */
	const uint32_t* southPiles_;
	uint32_t tileCount_;
};
