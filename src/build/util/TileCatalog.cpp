#include "TileCatalog.h"
#include <common/cli/Console.h>
#include <common/util/FileWriter.h>
#include "build/analyze/TileIndexBuilder.h"

void TileCatalog::build(TileIndexBuilder& builder)
{
	tileCount_ = builder.tileCount();
	levels_ = builder.zoomLevels();
	cellToPile_ = builder.takeCellToPile();
	tipToPile_ = builder.takeTipToPile();
	pileToTile_ = builder.takePileToTile();

	tileToPile_.reserve(tileCount_);
	for (int i = 1; i <= tileCount_; i++)
	{
		Console::msg("%d: %s", i, pileToTile_[i].toString().c_str());
		tileToPile_[pileToTile_[i]] = i;
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
