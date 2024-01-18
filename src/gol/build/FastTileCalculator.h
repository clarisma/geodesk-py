// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cassert>
#include <cstdint>
#include <memory>
#include "geom/Mercator.h"
#include "geom/Tile.h"


class FastTileCalculator
{
public:
	FastTileCalculator() :
		rows_(new uint16_t[LAT_LOOKUP_ROW_COUNT])
	{
		// Populate the lookup table that we use to convert 
		// from latitude to tile row
		int32_t lat100nd = Mercator::MIN_LAT_100ND;
		for (int i = 0; i < LAT_LOOKUP_ROW_COUNT; i++)
		{
			uint32_t row = Tile::rowFromYZ(Mercator::yFromLat100nd(lat100nd), ZOOM_LEVEL);
			assert(row < GRID_EXTENT);
			rows_[i] = static_cast<uint16_t>(row);
			lat100nd += RESOLUTION;
		}
	}

	uint32_t calculateCell(int32_t lon100nd, int32_t lat100nd) const
	{
		/*
		uint32_t latWrapped = static_cast<uint32_t>(lat100nd - Mercator::MAX_LAT_100ND);
		if(latWrapped >= Mercator::MAX_LAT_100ND - Mercator::MIN_LAT_100ND)
		*/
		if (lat100nd < Mercator::MIN_LAT_100ND || lat100nd > Mercator::MAX_LAT_100ND)
		{
			return GRID_CELL_COUNT;
		}
		uint32_t slot = static_cast<uint32_t>(
			(static_cast<int64_t>(lat100nd) - Mercator::MIN_LAT_100ND) 
				/ RESOLUTION);
		assert(slot < LAT_LOOKUP_ROW_COUNT);
		uint32_t row = rows_[slot];
		assert(row < GRID_EXTENT);
		uint32_t col = Tile::columnFromXZ(Mercator::xFromLon100nd(lon100nd), ZOOM_LEVEL);
		assert(col < GRID_EXTENT);
		return row * GRID_EXTENT + col;
	}

	static const int ZOOM_LEVEL = 12;
	static const uint32_t GRID_EXTENT = 1 << ZOOM_LEVEL;
	static const uint32_t GRID_CELL_COUNT = GRID_EXTENT * GRID_EXTENT;

private:
	static const uint32_t RESOLUTION = 32768;
	static constexpr uint32_t LAT_LOOKUP_ROW_COUNT =
		(Mercator::MAX_LAT_100ND - Mercator::MIN_LAT_100ND + RESOLUTION) /
		RESOLUTION;
	std::unique_ptr<uint16_t[]> rows_;
};