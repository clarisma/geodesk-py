// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include "TileScanner.h"

class TileLookupBuilder : public TileScanner<TileLookupBuilder>
{
public:
	TileLookupBuilder(const uint32_t* index, ZoomLevels levels) :
		TileScanner(index, levels)
	{
	}

	void build()
	{
		scan();
	}

	void tile(uint32_t parentTip, Tile tile, uint32_t tip)
	{
		char buf[80];
		tile.format(buf);
		printf("- %s\n", buf);
	}

	void omittedTile(uint32_t parentTip, Tile tile)
	{
	}
};
