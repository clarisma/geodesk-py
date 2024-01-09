// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <limits>
#include "Box.h"

typedef int32_t ZoomLevel;

class Tile
{
public:
	Tile() : tile_(-1) {}
	Tile(const Tile& other) : tile_(other.tile_) {}

	Tile& operator=(const Tile& other) 
	{
		tile_ = other.tile_;
		return *this;
	}

	operator uint32_t() const 
	{
		return tile_;
	}

	bool operator==(const Tile& other) const 
	{
		return tile_ == other.tile_;
	}

	inline static int columnFromXZ(int32_t x, ZoomLevel zoom) 
	{
		return (int)((static_cast<long long>(x) + (1LL << 31)) >> (32 - zoom));
	}

	inline static int rowFromYZ(int32_t y, ZoomLevel zoom)
	{
		return (int)((0x7fffffffLL - y) >> (32 - zoom));
	}

	int row() const
	{
		return (tile_ >> 12) & 0xfff;
	}

	int column() const
	{
		return tile_ & 0xfff;
	}

	inline int zoom() const
	{
		return (tile_ >> 24) & 0xf;
	}

	static inline Tile fromColumnRowZoom(int col, int row, ZoomLevel zoom)
	{
		return Tile(col, row, zoom);
	}

	Tile relative(int deltaCol, int deltaRow) const
	{
		return Tile(tile_ + (deltaRow << 12) + deltaCol);
	}

	int topY() const
	{
		return std::numeric_limits<int32_t>::max() - (row() << (32 - zoom()));
	}

	int bottomY() const
	{
		return std::numeric_limits<int32_t>::min() - (int)((int64_t)(row() + 1) << (32 - zoom()));
		// << 32 wraps around for int, that's why we cast to long
	}

	int leftX() const
	{
		int z = zoom();
		int col = column();
		return (col - (1 << (z - 1))) << (32 - z);
	}

	int rightX() const
	{
		int64_t extent = 1LL << (32 - zoom());
		return static_cast<int>(leftX() + extent - 1);
	}

	Box bounds() const
	{
		int z = zoom();
		int minX = leftX();
		int minY = bottomY();
		int64_t extent = 1LL << (32 - z);
		return Box(minX, minY, (int)(minX + extent - 1), (int)(minY + extent - 1));
	}

	/**
	 * Returns the tile number of an adjacent tile that lies
	 * in the specified direction.
	 *
     * @param  colDelta
     * @param  rowDelta
     * @return the tile number of the adjacent tile
     */
	Tile neighbor(int colDelta, int rowDelta)
	{
		int z = zoom();
		int x = column();
		int y = row();
		int mask = (1 << z) - 1;
		x = (x + colDelta) & mask;
		y = (y + rowDelta) & mask;
		return fromColumnRowZoom(x, y, z);
	}

	std::string toString() const
	{
		char buf[80];
		sprintf(buf, "%d/%d/%d", zoom(), column(), row());
		return std::string(buf);
	}

private:
	inline Tile(int col, int row, ZoomLevel zoom)
		: tile_((zoom << 24) | (row << 12) | col) {}
	
	inline Tile(int t) : tile_(t) {}
	
	uint32_t tile_;
};


namespace std 
{
	template<>
	struct hash<Tile> 
	{
		size_t operator()(const Tile& tile) const 
		{
			return std::hash<uint32_t>()(tile); 
		}
	};
}