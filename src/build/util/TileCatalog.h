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

	void build(int tileCount, const uint32_t* index, ZoomLevels levels);

	int tileCount() const { return tileCount_; }

	Tile tileOfPile(int pile) const
	{
		return pileToTile_[pile];
	}

	int pileOfCoordinate(Coordinate c) const
	{
		int col = Tile::columnFromXZ(c.x, MAX_ZOOM);
		int row = Tile::rowFromYZ(c.y, MAX_ZOOM);
		// printf("12/%d/%d\n", col, row);
		return grid_[cellOf(col, row)];
	}

private:
	class Builder : TileIndexScanner<Builder>
	{
	public:
		Builder(const uint32_t* index, ZoomLevels levels, int officialTileCount) :
			TileIndexScanner(index, levels),
			officialTileCount_(officialTileCount),
			tileCount_(0)
		{
		}

		void build();
		void branchTile(uint32_t parentTip, Tile tile, uint32_t tip);
		void leafTile(uint32_t parentTip, Tile tile, uint32_t tip);
		void omittedTile(uint32_t parentTip, Tile tile);
		std::unique_ptr<const int[]> takeGrid() { return std::move(grid_); }
		std::unique_ptr<const int[]> takeTipToPile() { return std::move(tipToPile_); }
		std::unique_ptr<const Tile[]> takePileToTile() { return std::move(pileToTile_); }
		int tileCount() const { return tileCount_; }

	private:
		void fillGrid(Tile tile, int pile);

		std::unique_ptr<int[]> grid_;
		std::unique_ptr<int[]> tipToPile_;
		std::unique_ptr<Tile[]> pileToTile_;
		int officialTileCount_;
		int tileCount_;
	};

	/*
	struct PileEntry
	{
		Tile tile;
		uint32_t parentPile;
	};
	*/

	static constexpr int cellOf(int col, int row)
	{
		assert(col >= 0 && col < (1 << MAX_ZOOM));
		assert(row >= 0 && row < (1 << MAX_ZOOM));
		return row * (1 << MAX_ZOOM) + col;
	}

	std::unique_ptr<const int[]> grid_;
	std::unique_ptr<const int[]> tipToPile_;
	std::unique_ptr<const Tile[]> pileToTile_;
	int tileCount_;
};
