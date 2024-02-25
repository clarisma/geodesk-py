// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <common/util/Bits.h>
#include "Tile.h"

class TilePair : public Tile
{
public:
	static const uint32_t EXTENDS_EAST_BIT = 28;
	static const uint32_t EXTENDS_SOUTH_BIT = 29;
	static const uint32_t EXTENDS_EAST = 1 << EXTENDS_EAST_BIT;
	static const uint32_t EXTENDS_SOUTH = 1 << EXTENDS_SOUTH_BIT;

	TilePair(Tile tile) : Tile(tile) {}

	bool extendsEast() const
	{
		return tile_ & EXTENDS_EAST;
	}

	bool extendsSouth() const
	{
		return tile_ & EXTENDS_SOUTH;
	}

	bool hasSecond() const
	{
		return tile_ & (EXTENDS_EAST | EXTENDS_SOUTH);
	}

	Tile first() const
	{
		return Tile(tile_ & 0x0fff'ffff);
	}

	Tile second() const
	{
		return first().relative(
			(tile_ >> EXTENDS_EAST_BIT) & 1,
			tile_ >> EXTENDS_SOUTH_BIT);
	}

	TilePair& operator+=(const Tile& tile)
	{
		int colA = column() << (BASE_ZOOM - zoom());
		int rowA = row() << (BASE_ZOOM - zoom());
		int colB = tile.column() << (BASE_ZOOM - tile.zoom());
		int rowB = tile.row() << (BASE_ZOOM - tile.zoom());
		int minCol = std::min(colA, colB);
		int maxCol = std::max(colA, colB);
		int minRow = std::min(rowA, rowB);
		int maxRow = std::max(rowA, rowB);
		int newZoom = std::min(
			highestPairZoom(minCol, maxCol),
			highestPairZoom(minRow, maxRow));
		int zoomDelta = BASE_ZOOM - newZoom;
		minCol >>= zoomDelta;
		minRow >>= zoomDelta;
		maxCol >>= zoomDelta;
		maxRow >>= zoomDelta;
		if ((maxCol - minCol) & (maxRow - minRow))
		{
			zoomDelta = std::min(
				commonZoomDelta(minCol, maxCol),
				commonZoomDelta(minRow, maxRow));
			newZoom -= zoomDelta;
			minCol >>= zoomDelta;
			minRow >>= zoomDelta;
			maxCol >>= zoomDelta;
			maxRow >>= zoomDelta;
		}
		tile_ = Tile::fromColumnRowZoom(minCol, minRow, newZoom) |
			((maxCol - minCol) << EXTENDS_EAST_BIT) |
			((maxRow - minRow) << EXTENDS_SOUTH_BIT);
		return *this;
	}

private:
	static const int BASE_ZOOM = 16;

	/**
	 * Calculates the highest zoom level at which the delta between
	 * at min and max is <= 1
	 * 
	 * @param min   lower column or row (at base zoom level)
	 * @param max   hgiher column or row (at base zoom level)
	 */
	static constexpr int highestPairZoom(int min, int max)
	{
		int w = max - min;
		int modW = w | 1;
		int zoom = BASE_ZOOM - 31 + Bits::countLeadingZerosInNonZero32(modW);
		int shift = BASE_ZOOM - zoom;
		int mask = (1 << shift) - 1;
		int offset = min & mask;
		return zoom - ((w + offset) >> (shift + 1));
	}

	/**
	 * Calculates the zoom delta needed to arrive at the zoom level
	 * where min and max are equal
	 */
	static constexpr int commonZoomDelta(int min, int max)
	{
		int v = min ^ max;
		return v ? (32 - Bits::countLeadingZeros32(v)) : 0;
	}
};

