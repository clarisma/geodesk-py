// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cassert>
#include <cstdint>
#include <filesystem>
#include <unordered_map>
#include "feature/ZoomLevels.h"
#include "geom/Tile.h"

class TileIndexBuilder;

class TileCatalog
{
public:
	static const int MAX_ZOOM = 12;

	void build(TileIndexBuilder& builder);

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
		assert(cellToPile_[cellOf(col, row)] == pileOfTileOrParent(
			Tile::fromColumnRowZoom(col, row, MAX_ZOOM)));
		return cellToPile_[cellOf(col, row)];
	}

	int pileOfTile(Tile tile) const noexcept
	{
		auto it = tileToPile_.find(tile);
		return (it != tileToPile_.end()) ? it->second : 0;
	}

	int pileOfTileOrParent(Tile tile) const noexcept;

	void write(std::filesystem::path path) const;

private:
	static constexpr int cellOf(int col, int row)
	{
		assert(col >= 0 && col < (1 << MAX_ZOOM));
		assert(row >= 0 && row < (1 << MAX_ZOOM));
		return row * (1 << MAX_ZOOM) + col;
	}

	std::unique_ptr<const int[]> cellToPile_;
	std::unique_ptr<const int[]> tipToPile_;
	std::unique_ptr<const Tile[]> pileToTile_;
	std::unordered_map<Tile, int> tileToPile_;
	int tileCount_;
	ZoomLevels levels_;
};
