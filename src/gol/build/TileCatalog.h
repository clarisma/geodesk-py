// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cassert>
#include <cstdint>
#include "geom/Tile.h"

class TileCatalog
{
public:
	Tile tileOfPile(uint32_t pile) const
	{
		return piles_[pile].tile;
	}


private:
	struct PileEntry
	{
		Tile tile;
		uint32_t parentPile;
	};

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
