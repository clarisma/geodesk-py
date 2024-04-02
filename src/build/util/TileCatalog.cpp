#include "TileCatalog.h"
#include <common/cli/Console.h>
#include <common/util/FileWriter.h>

void TileCatalog::build(int tileCount, const uint32_t* index, ZoomLevels levels)
{
	levels_ = levels;
	tileCount_ = tileCount;
	Builder builder(index, levels, tileCount);
	builder.build();
	grid_ = builder.takeGrid();
	tipToPile_ = builder.takeTipToPile();
	pileToTile_ = builder.takePileToTile();
	assert(builder.tileCount() == tileCount);

	tileToPile_.reserve(tileCount);
	for (int i = 1; i <= tileCount; i++)
	{
		Console::msg("%d: %s", i, pileToTile_[i].toString().c_str());
		tileToPile_[pileToTile_[i]] = i;
	}
}


void TileCatalog::Builder::build()
{
	int extent = 1 << MAX_ZOOM;
	grid_ = std::make_unique<int[]>(extent * extent);
		// TODO: no 0-initialization needed, every cell should be filled
		
	uint32_t tipCount = index()[0];
	tipToPile_ = std::make_unique<int[]>(tipCount+1);		
		// only needed here as not all TIPs are valid
	pileToTile_ = std::make_unique<Tile[]>(officialTileCount_ + 1);
		// TODO: no 0-initialization needed, every cell should be filled

	scan();
	assert(tileCount_ == officialTileCount_);

	for (int i = 0; i < extent * extent; i++)
	{
		assert(grid_[i] != 0);
	}
}

void TileCatalog::Builder::branchTile(uint32_t parentTip, Tile tile, uint32_t tip)
{
	uint32_t pile = ++tileCount_;
	tipToPile_[tip] = pile;
	pileToTile_[pile] = tile;
}

void TileCatalog::Builder::leafTile(uint32_t parentTip, Tile tile, uint32_t tip)
{
	uint32_t pile = ++tileCount_;
	tipToPile_[tip] = pile;
	pileToTile_[pile] = tile;
	fillGrid(tile, pile);
}

void TileCatalog::Builder::omittedTile(uint32_t parentTip, Tile tile)
{
	fillGrid(tile, tipToPile_[parentTip]);
}

void TileCatalog::Builder::fillGrid(Tile tile, int pile)
{
	// printf("Filling %s with %d...\n", tile.toString().c_str(), pile);
	int step = MAX_ZOOM - tile.zoom();
	int extent = 1 << step;
	int left = tile.column() << step;
	int right = left + extent - 1;
	int top = tile.row() << step;
	int bottom = top + extent - 1;
	for (int row = top; row <= bottom; row++)
	{
		for (int col = left; col <= right; col++)
		{
			grid_[cellOf(col, row)] = pile;
		}
	}
}


int TileCatalog::pileOfTileOrParent(Tile tile) const noexcept
{
	for (;;)
	{
		auto it = tileToPile_.find(tile);
		if (it != tileToPile_.end()) return it->second;
		int zoom = tile.zoom();
		assert(zoom > 0); // root tile must be in tileToPile_
		tile = tile.zoomedOut(levels_.parentZoom(zoom));
	}
}


void TileCatalog::write(std::filesystem::path path) const
{
	FileWriter out(path);
	for (int i = 1; i <= tileCount_; i++)
	{
		out << pileToTile_[i] << '\n';
	}
}
