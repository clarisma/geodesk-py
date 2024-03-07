#include "TileCatalog.h"

void TileCatalog::build(int tileCount, const uint32_t* index, ZoomLevels levels)
{
	tileCount_ = tileCount;
	Builder builder(index, levels, tileCount);
	builder.build();
	assert(_CrtCheckMemory());
	grid_ = builder.takeGrid();
	tipToPile_ = builder.takeTipToPile();
	pileToTile_ = builder.takePileToTile();
	assert(builder.tileCount() == tileCount);
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
