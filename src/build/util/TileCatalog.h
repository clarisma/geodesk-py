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

	void build(const uint32_t* index, ZoomLevels levels);

	Tile tileOfPile(uint32_t pile) const
	{
		return piles_[pile].tile;
	}

	uint32_t pileOfCoordinate(Coordinate c) const
	{
		int col = Tile::columnFromXZ(c.x, MAX_ZOOM);
		int row = Tile::rowFromYZ(c.y, MAX_ZOOM);
		return grid_.get()[cellOf(col, row)];
	}

private:
	class Builder : TileIndexScanner<Builder>
	{
	public:
		Builder(const uint32_t* index, ZoomLevels levels) :
			TileIndexScanner(index, levels)
		{
		}

		const uint32_t* build();
		void tile(uint32_t parentTip, Tile tile, uint32_t tip);
		void omittedTile(uint32_t parentTip, Tile tile);

	private:
		std::unique_ptr<uint32_t[]> grid_;
	};

	struct PileEntry
	{
		Tile tile;
		uint32_t parentPile;
	};

	static constexpr int cellOf(int col, int row)
	{
		return row * (1 << MAX_ZOOM) + col;
	}

	std::unique_ptr<const uint32_t[]> grid_;

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
};
