#include "TileIndexBuilder.h"
#include <unordered_map>

TileIndexBuilder::TileIndexBuilder(const TileSettings& settings) :
	arena_(64 * 1024),
	settings_(settings)
{
	tiles_.reserve(settings.maxTiles);	// TODO: allocate more?
}

void TileIndexBuilder::build(const uint32_t* nodeCounts)
{
	createLeafTiles(nodeCounts);
	addParentTiles();
	if (tiles_.size() > settings_.maxTiles)
	{
		// If there are too many tiles, keep only the largest

		std::sort(tiles_.begin(), tiles_.end(), [](const STile* a, const STile* b)
			{
				return a->count() > b->count();  // Use > for descending order
			});
		tiles_.erase(tiles_.begin() + settings_.maxTiles);
	}
}

void TileIndexBuilder::createLeafTiles(const uint32_t* nodeCounts)
{
	int extent = 1 << settings_.leafZoomLevel;
	const uint32_t* p = nodeCounts;
	for (int row = 0; row < extent; row++)
	{
		for (int col = 0; col < extent; col++)
		{
			uint32_t nodeCount = *p++;
			if (nodeCount == 0) continue;
			// We skip empty tiles, but for now we need to track tiles with
			// a node count below the minimum, so we can add that count
			// to the next-lower tile in the pyramid
			tiles_.push_back(arena_.create<STile>(
				Tile::fromColumnRowZoom(col, row, settings_.leafZoomLevel),
				nodeCount));
		}
	}
}
	

void TileIndexBuilder::addParentTiles()
{
	std::unordered_map<Tile, STile*> parentTiles;
	std::vector<STile*> childTiles;
	std::vector<STile*>* pChildTiles = &tiles_;

	int parentZoom = settings_.leafZoomLevel;
	for (;;)
	{
		while (!settings_.zoomLevels.isValidZoomLevel(parentZoom)) parentZoom--;
		if (parentZoom == 0) break;
		for (STile* ct : *pChildTiles)
		{
			Tile parentTile = ct->tile().zoomedOut(parentZoom);
			STile* pt;
			auto it = parentTiles.find(parentTile);
			if (it == parentTiles.end())
			{
				pt = arena_.create<STile>(parentTile, 0);
				parentTiles[parentTile] = pt;
			}
			else
			{
				pt = it->second;
			}
			// We don't actually add the child to the parent yet,
			// we just mark the parent
			ct->setParent(pt);
			pt->addCount(ct);
		}
		childTiles.clear();
		for (const auto& it : parentTiles)
		{
			STile* pt = it.second;
			childTiles.push_back(pt);
			if (pt->count() >= settings_.minTileDensity)
			{
				// Add parent tile to the main tile collection
				// only if it meets the minimum node count
				tiles_.push_back(pt);
			}
		}
		pChildTiles = &childTiles;
		parentZoom--;
	}
	// Create the root tile (set its node count to the maximum 
	// to ensure it is always included in the tile pyramid)
	tiles_.push_back(arena_.create<STile>(Tile::fromColumnRowZoom(0,0,0), 
		std::numeric_limits<uint64_t>::max()));
}
