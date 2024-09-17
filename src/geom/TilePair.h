// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "geom/Tile.h"

class TilePair : public Tile
{
public:
	static const uint32_t EXTENDS_EAST_BIT = 28;
	static const uint32_t EXTENDS_SOUTH_BIT = 29;
	static const uint32_t EXTENDS_EAST = 1 << EXTENDS_EAST_BIT;
	static const uint32_t EXTENDS_SOUTH = 1 << EXTENDS_SOUTH_BIT;

	TilePair() {}
	TilePair(Tile tile) : Tile(tile) {}
	TilePair(const TilePair& other) : Tile(other.tile_) {}

	TilePair& operator=(const TilePair& other)
	{
		tile_ = other.tile_;
		return *this;
	}

	void extend(int bits)
	{
		assert((bits & 3) == bits);
		tile_ |= bits << EXTENDS_EAST_BIT;
	}

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

	/**
	 * Returns the Tile that is the southern/eastern twin in the TilePair,
	 * or the single Tile if this is not a true pair
	 */
	Tile second() const
	{
		return first().relative(
			(tile_ >> EXTENDS_EAST_BIT) & 1,
			tile_ >> EXTENDS_SOUTH_BIT);
	}

	
	TilePair& operator+=(const TilePair& other)
	{
		*this += other.first();
		if (other.hasSecond()) *this += other.second();
		return *this;
	}

	// TODO: replace this with non-branching
	TilePair& operator+=(const Tile& other)
	{
		if (isNull())
		{
			tile_ = other;
			return *this;
		}

		int commonZoom = std::min(zoom(), other.zoom());
		TilePair pair = zoomedOut(commonZoom);
		int otherDelta = other.zoom() - commonZoom;
		

		uint32_t aCol = static_cast<uint32_t>(pair.column());
		uint32_t aRow = static_cast<uint32_t>(pair.row());
		uint32_t bCol = static_cast<uint32_t>(other.column()) >> otherDelta;
		uint32_t bRow = static_cast<uint32_t>(other.row()) >> otherDelta;

		uint32_t left = std::min(aCol, bCol);
		uint32_t top = std::min(aRow, bRow);
		uint32_t right = std::max(aCol + ((pair.tile_ >> EXTENDS_EAST_BIT) & 1), bCol);
		uint32_t bottom = std::max(aRow + ((pair.tile_ >> EXTENDS_SOUTH_BIT) & 1), bRow);

		for (;;)
		{
			uint32_t colDelta = right - left;
			uint32_t rowDelta = bottom - top;
			if ((colDelta == 0 || rowDelta == 0) && colDelta <= 1 && rowDelta <= 1)
			{
				tile_ = Tile::fromColumnRowZoom(left, top, commonZoom)
					| ((right - left) << EXTENDS_EAST_BIT)
					| ((bottom - top) << EXTENDS_SOUTH_BIT);
				return *this;
			}
			left >>= 1;
			right >>= 1;
			top >>= 1;
			bottom >>= 1;
			commonZoom--;
		}
	}
		

	/*
	// Goal: Find highest zoom level where col or row diverge by no more than 1
	// TODO: broken!!!!
	TilePair& operator+=(const Tile& tile)
	{
		if (isNull())
		{
			tile_ = tile;
			return *this;
		}
		// First, we normalize each tile's col/row to zoom level 32
		// (We need to cast to unsigned to avoid sign extension later)

		uint32_t aCol = static_cast<uint32_t>(column()) << (32 - zoom());
		uint32_t aRow = static_cast<uint32_t>(row()) << (32 - zoom());
		uint32_t bCol = static_cast<uint32_t>(tile.column()) << (32 - tile.zoom());
		uint32_t bRow = static_cast<uint32_t>(tile.row()) << (32 - tile.zoom());

		uint32_t left = std::min(aCol, bCol);
		uint32_t top = std::min(aRow, bRow);
		uint32_t right = std::min(aCol + ((tile_ >> EXTENDS_EAST_BIT) & 1), bCol);
		uint32_t bottom = std::min(aRow + ((tile_ >> EXTENDS_SOUTH_BIT) & 1), bRow);

		// Then we'll count how many bits the cols and rows have in common
		// (we set bit 0 to 1 so we can use the faster bitcount that only works
		// on non-zero values)
		// We want to find the zoom level at which the rows or cols (but not both)
		// differ by at most one
		// If both cols and rows diverge at same level, that's the common zoom
		// Otherwise, we use one level below the *lower* of the zoom levels
		int commonColZoom = Bits::countLeadingZerosInNonZero32((right - left) | 1);
		int commonRowZoom = Bits::countLeadingZerosInNonZero32((bottom - top) | 1);
		int commonZoom = std::min(commonColZoom, commonRowZoom);

		// The zoom level cannot be higher than the lower of the tile pair's zoom or
		// the zoom of the tile to be added
		int z = std::min(std::min(zoom(), tile.zoom()), commonZoom);
		int shift = 32 - z; 

		// Now we adjust the col/row values to the target zoom, choose the lower
		// values as the col/row of the first tile, and set the flags to indicate
		// the second tile (if any) of the pair
		// (These shifts are unsigned)
		// We have to upcast to uint64_t because shifting uint32_t by 32 bits 
		// is undefined
		left = static_cast<uint32_t>(static_cast<uint64_t>(left) >> shift);
		right = static_cast<uint32_t>(static_cast<uint64_t>(right) >> shift);
		top = static_cast<uint32_t>(static_cast<uint64_t>(top) >> shift);
		bottom = static_cast<uint32_t>(static_cast<uint64_t>(bottom) >> shift);
		assert(right >= left);
		assert(bottom >= top);
		assert(right-left <= 1);
		assert(bottom-top <= 1);
		tile_ = Tile::fromColumnRowZoom(left, top, z)
			| ((right - left) << EXTENDS_EAST_BIT)
			| ((bottom - top) << EXTENDS_SOUTH_BIT);
		return *this;
	}
	*/


	TilePair zoomedOut(int lowerZoom) const
	{
		assert(lowerZoom <= this->zoom()); // Can't zoom out to higher level
		Tile northWest = first().zoomedOut(lowerZoom);
		Tile southEast = second().zoomedOut(lowerZoom);
		return TilePair(northWest | ((northWest == southEast) ? 0 :
			tile_ & (EXTENDS_EAST | EXTENDS_SOUTH)));
	}

	void format(char* buf) const
	{
		Format::unsafe(buf, "%d/%d%s/%d%s", zoom(), 
			column(), (extendsEast() ? "+" : ""),
			row(), (extendsSouth() ? "+" : ""));
	}

	std::string toString() const
	{
		char buf[80];
		format(buf);
		return std::string(buf);
	}

	/**
	 * 0 = the "pair" is really a single and represents the Tile itself
	 * 1 = the pair covers the Tile and a tile to the north
	 * 2 = the pair covers the Tile and a tile to the west
	 * 3 = dto. south
	 * 4 = dto. east
	 * 7 = the TilePair does not cover the given Tile
	 */
	uint_fast8_t isTwinOf(Tile other)
	{
		assert(zoom() == other.zoom());
		uint_fast32_t colDelta = static_cast<uint_fast32_t>(other.column() - column());
		uint_fast32_t rowDelta = static_cast<uint_fast32_t>(other.row() - row());
		if (colDelta > 1 || rowDelta > 1) return Twin::INVALID_TWIN;

		/*
		if (rowDelta == 1)
		{
			printf("!!!");
		}
		*/

		uint_fast32_t shift =
			((tile_ >> (EXTENDS_EAST_BIT - 4)) & 0x30) |
			(rowDelta << 3) | (colDelta << 2);
			//already x4 to address nibble within twins

		//                                 A98'7654'3210
		constexpr uint64_t twins = 0x7777'7173'7724'7770;
			// 0000 (0) = self/no twin (0)
			// 1010 (10) = north twin (1)
			// 0101 (5) = west twin  (2)
			// 1000 (8) = south twin (3)
			// 0100 (6) = east twin  (4)
			// all others are invalid (7)

		uint_fast8_t twinCode = static_cast<uint_fast8_t>((twins >> shift) & 15);
		assert(twinCode == Twin::INVALID_TWIN || TilePair::covering(other, twinCode) == *this);
		return twinCode;
	}

	static TilePair covering(Tile tile, uint_fast8_t twinCode)
	{
		assert(twinCode >= 0 && twinCode <= 4);

		//                                4    3    2    1    0      
		constexpr uint32_t offsets = 0b0100'1000'0101'1010'0000;
		uint_fast32_t nibble = offsets >> (twinCode * 4);
		int colDelta = static_cast<int>(nibble & 1);
		int rowDelta = static_cast<int>((nibble >> 1) & 1);
		uint_fast32_t southEastBits = (nibble & 12) << (EXTENDS_EAST_BIT - 2);
		TilePair pair = TilePair(fromColumnRowZoom(
			tile.column() - colDelta, tile.row() - rowDelta, tile.zoom()) |
			southEastBits);
		// assert(pair.isTwinOf(tile) == twinCode);
		return pair;
	}

private:
	TilePair(uint32_t bits) { tile_ = bits; }
};

