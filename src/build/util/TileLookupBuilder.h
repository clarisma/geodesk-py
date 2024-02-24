// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include "TileIndexScanner.h"

class TileLookupBuilder : public TileIndexScanner<TileLookupBuilder>
{
public:
	static const int MAX_ZOOM = 12;

	TileLookupBuilder(const uint32_t* index, ZoomLevels levels) :
		TileIndexScanner(index, levels)
	{
	}

	void build()
	{
		int extent = 1 << MAX_ZOOM;
		grid_.reset(new uint32_t[extent * extent]);
		scan();
	}

	void tile(uint32_t parentTip, Tile tile, uint32_t tip)
	{
		char buf[80];
		tile.format(buf);
		// printf("- %s\n", buf);
	}

	void omittedTile(uint32_t parentTip, Tile tile)
	{
		char buf[80];
		tile.format(buf);
		// printf("- %s (omitted)\n", buf);
	}

private:
	std::unique_ptr<uint32_t[]> grid_;
};
