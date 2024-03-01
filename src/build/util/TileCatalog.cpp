#include "TileCatalog.h"

void TileCatalog::build(uint32_t tileCount, const uint32_t* index, ZoomLevels levels)
{
	tileCount_ = tileCount;
	Builder builder(index, levels);
	builder.build();
	grid_.reset(builder.takeGrid());
	tipToPile_.reset(builder.takeTipToPile());
	assert(builder.tileCount() == tileCount);
}


void TileCatalog::Builder::build()
{
	int extent = 1 << MAX_ZOOM;
	uint32_t* grid = new uint32_t[extent * extent];
	memset(grid, 0, sizeof(uint32_t) * extent * extent);
		// TODO: not needed, every cell should be filled
	grid_.reset(grid);
	
	uint32_t tipCount = index()[0];
	uint32_t* table = new uint32_t[tipCount + 1];
	memset(table, 0, sizeof(uint32_t) * (tipCount + 1));
	tipToPile_.reset(table);

	scan();

	for (int i = 0; i < extent * extent; i++)
	{
		assert(grid[i]);
	}
}

void TileCatalog::Builder::branchTile(uint32_t parentTip, Tile tile, uint32_t tip)
{
	uint32_t pile = ++tileCount_;
	tipToPile_.get()[tip] = pile;
}

void TileCatalog::Builder::leafTile(uint32_t parentTip, Tile tile, uint32_t tip)
{
	uint32_t pile = ++tileCount_;
	tipToPile_.get()[tip] = pile;
	fillGrid(tile, pile);
}

void TileCatalog::Builder::omittedTile(uint32_t parentTip, Tile tile)
{
	fillGrid(tile, tipToPile_.get()[parentTip]);
}

void TileCatalog::Builder::fillGrid(Tile tile, uint32_t pile)
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
			grid_.get()[cellOf(col, row)] = pile;
		}
	}
}
