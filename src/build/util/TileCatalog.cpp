#include "TileCatalog.h"

void TileCatalog::build(const uint32_t* index, ZoomLevels levels)
{
	Builder builder(index, levels);
	grid_.reset(builder.build());
}


const uint32_t* TileCatalog::Builder::build()
{
	int extent = 1 << MAX_ZOOM;
	grid_.reset(new uint32_t[extent * extent]);
	scan();
	return grid_.release();
}

void TileCatalog::Builder::tile(uint32_t parentTip, Tile tile, uint32_t tip)
{
	if (tile.zoom() == MAX_ZOOM)
	{
		grid_.get()[cellOf(tile.column(), tile.row())] = tip;
		// TODO: tip or pile?
	}
}

void TileCatalog::Builder::omittedTile(uint32_t parentTip, Tile tile)
{
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
			grid_.get()[cellOf(col, row)] = parentTip;
			// TODO: tip or pile?
		}
	}
}
