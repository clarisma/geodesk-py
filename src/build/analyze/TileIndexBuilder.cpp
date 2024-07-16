#include "TileIndexBuilder.h"
#include <unordered_map>
#include <common/cli/Console.h>


struct STile
{
	STile* next;
	Tile tile;
	int tip;
	int maxChildCount;
	int childCount;
	uint64_t totalNodeCount;
	uint64_t estimatedTileSize;
	STile* parent;
	STile* children[1];
};


TileIndexBuilder::TileIndexBuilder(const BuildSettings& settings) :
	arena_(64 * 1024),
	settings_(settings),
	tierCount_(0),
	tileCount_(0),
	pileCount_(0),
	maxZoom_(settings.leafZoomLevel())
{
	ZoomLevels levels = settings.zoomLevels();
	ZoomLevels::Iterator iter = levels.iter();
	for (;;)
	{
		int zoom = iter.next();
		if (zoom < 0) break;
		Tier& tier = tiers_[tierCount_++];
		tier.level = zoom;
		tier.skippedLevels = levels.skippedAfterLevel(zoom);
		tier.firstTile = nullptr;
		tier.tileCount = 0;
	}
}

uint32_t TileIndexBuilder::sumTileCounts() noexcept
{
	tileCount_ = 0;
	for (int i = 0; i < tierCount_; i++)
	{
		tileCount_ += tiers_[i].tileCount;
	}
	return tileCount_;
}

void TileIndexBuilder::build(std::unique_ptr<const uint32_t[]> nodeCounts)
{
	createLeafTiles(nodeCounts.get());
	addParentTiles();
	trimTiles();
	linkChildTiles();
	int tipCount = assignTip(tiers_[0].firstTile, 1);
	tileIndex_.reset(new uint32_t[tipCount]);
	tileIndex_[0] = tipCount-1;
	tipToPile_ = std::make_unique<int[]>(tipCount);
	pileToTile_.reset(new Tile[tileCount_ + 1]);
	tileSizeEstimates_.reset(new uint32_t[tileCount_ + 1]);
	tileSizeEstimates_[0] = 0;
	cellToPile_.reset(new int[1 << (maxZoom_ * 2)]);

	buildTile(tiers_[0].firstTile);

	Console::msg("- %u tiles", tileCount_);
	Console::msg("- %u TIPs", tileIndex_[0]);
}

void TileIndexBuilder::trimTiles()
{
	uint32_t tileCount = sumTileCounts();
	if (tileCount > settings_.maxTiles())
	{
		// If there are too many tiles, keep only the largest

		STile** pTiles = arena_.allocArray<STile*>(tileCount);
		STile** p = pTiles;
		for (int i = 0; i < tierCount_; i++)
		{
			Tier::Iterator iter = tiers_[i].iter();
			for (;;)
			{
				STile* t = iter.next();
				if (!t) break;
				*p++ = t;
			}
		}

		STile** pEnd = pTiles + tileCount;
		std::sort(pTiles, pEnd, [](const STile* a, const STile* b)
			{
				return a->totalNodeCount > b->totalNodeCount;  // Use > for descending order
			});
		p = pTiles + settings_.maxTiles();
		do
		{
			(*p++)->totalNodeCount = 0;
		}
		while (p < pEnd);
		tileCount_ = settings_.maxTiles();
	}
}

void TileIndexBuilder::createLeafTiles(const uint32_t* nodeCounts)
{
	int extent = 1 << maxZoom_;
	const uint32_t* p = nodeCounts;
	STile* tile = nullptr;
	uint32_t count = 0;
	for (int row = 0; row < extent; row++)
	{
		for (int col = 0; col < extent; col++)
		{
			uint32_t nodeCount = *p++;
			if (nodeCount == 0) continue;
			// We skip empty tiles, but for now we need to track tiles with
			// a node count below the minimum, so we can add that count
			// to the next-lower tile in the pyramid
			tile = createTile(
				Tile::fromColumnRowZoom(col, row, maxZoom_),
				0, nodeCount, tile);
			count++;
		}
	}

	Tier* tier = &tiers_[tierCount_ - 1];
	if (tier->level < maxZoom_)
	{
		// If the tile pyramid does not include zoom level 12,
		// we create a fake tier to store the level-12 tiles

		// TODO: In that case, the Analyzer should create a coarser grid instead

		tier = &tiers_[tierCount_];
		tier->level = maxZoom_;
		tier->skippedLevels = 0;
	}
	tier->firstTile = tile;
	// printf("firstTile of zoom %d = %p\n", tier->level, tile);
	tier->tileCount = count;
}
	
STile* TileIndexBuilder::createTile(
	Tile tile, int maxChildCount, uint64_t nodeCount, STile* next)
{
	size_t size = sizeof(STile) + (maxChildCount - 1) * sizeof(STile*);
	STile* pt = arena_.allocWithExplicitSize<STile>(size);
	memset(pt, 0, size);
	pt->next = next;
	pt->tile = tile;
	pt->maxChildCount = maxChildCount;
	pt->totalNodeCount = nodeCount;
	pt->estimatedTileSize = nodeCount * ESTIMATED_BYTES_PER_NODE;
	return pt;
}


void TileIndexBuilder::addParentTiles()
{
	std::unordered_map<Tile, STile*> parentTileMap;
	uint32_t minTileDensity = settings_.minTileDensity();
	
	int startTier = tiers_[tierCount_-1].level < maxZoom_ ?
		tierCount_ : (tierCount_ - 1);
	startTier = std::max(startTier, 1);
	
	for (int i = startTier; i > 0; i--)
	{
		Tier& childTier = tiers_[i];
		Tier& parentTier = tiers_[i-1];
		int parentZoom = parentTier.level;
		uint32_t maxChildCount = parentTier.maxChildCount();
	
		Tier::Iterator iter = childTier.iter();
		childTier.clearTiles();
		STile* firstParentTile = nullptr;
		for (;;)
		{
			STile* ct = iter.next();
			if (!ct) break;
			Tile parentTile = ct->tile.zoomedOut(parentZoom);
			STile* pt;
			auto it = parentTileMap.find(parentTile);
			if (it == parentTileMap.end())
			{
				pt = createTile(parentTile, maxChildCount, 0, firstParentTile);
				parentTileMap[parentTile] = pt;
				parentTier.addTile(pt);
				//printf("Created parent tile %s\n", parentTile.toString().c_str());
			}
			else
			{
				pt = it->second;
			}
			// We don't actually add the child to the parent yet,
			// we just mark the parent
			ct->parent = pt;
			pt->totalNodeCount += ct->totalNodeCount;
			pt->estimatedTileSize += ct->estimatedTileSize /
				ESTIMATED_CHILD_BYTES_PER_PARENT_BYTE;
			if (ct->totalNodeCount >= minTileDensity)
			{
				childTier.addTile(ct);
			}
			else
			{
				pt->estimatedTileSize += ct->estimatedTileSize;
			}
			ct = ct->next;
		}
		parentTileMap.clear();
	}
	assert(tiers_[0].tileCount == 1);
}


void TileIndexBuilder::linkChildTiles()
{
	for(int i=1; i<tierCount_; i++)
	{
		Tier& tier = tiers_[i];
		Tier::Iterator iter = tier.iter();
		tier.clearTiles();
		for(;;)
		{
			STile* tile = iter.next();
			if (!tile) break;
			STile* parent = tile->parent;
			assert(parent);
			if (tile->totalNodeCount)
			{
				addChildTile(parent, tile);
				tier.addTile(tile);
			}
			else
			{
				uint64_t estimatedChildSize = tile->estimatedTileSize;
				parent->estimatedTileSize += estimatedChildSize +
					estimatedChildSize / ESTIMATED_CHILD_BYTES_PER_PARENT_BYTE;
			}
			tile = tile->next;
		}
	}
}


void TileIndexBuilder::Tier::addTile(STile* tile)
{
	tile->next = firstTile;
	firstTile = tile;
	tileCount++;
}


int TileIndexBuilder::assignTip(STile* tile, int tip) noexcept
{
	tile->tip = tip;
	int childTip = tip + ((tile->maxChildCount == 64) ? 3 : 2);
	int nextTip = childTip + tile->childCount;
	for (int i = 0; i < tile->maxChildCount; i++)
	{
		STile* childTile = tile->children[i];
		if (childTile)
		{
			if (childTile->childCount)
			{
				nextTip = assignTip(childTile, nextTip);
			}
			else
			{
				childTile->tip = childTip;
			}
			childTip++;
		}
	}
	return nextTip;
}


void TileIndexBuilder::addChildTile(STile* parent, STile* child)
{
	assert(parent->childCount < parent->maxChildCount);

	int childZoom = child->tile.zoom();
	int step = childZoom - parent->tile.zoom();
	assert(step > 0 && step <= 3);
	int left = parent->tile.column() << step;
	int top = parent->tile.row() << step;
	int width = 1 << step;
	int pos = (child->tile.row() - top) * width + (child->tile.column() - left);
	assert(pos >= 0 && pos < parent->maxChildCount);
	assert(parent->children[pos] == nullptr);
	parent->children[pos] = child;
	parent->childCount++;
}


void TileIndexBuilder::buildTile(STile* tile) noexcept
{
	assert(tile->tip);
	pileCount_++;
	int pile = pileCount_;
	tipToPile_[tile->tip] = pile;
	pileToTile_[pile] = tile->tile;

	uint32_t pageSize = settings_.featurePilesPageSize();
	uint32_t pages = (tile->estimatedTileSize + pageSize - 1) / pageSize;
	tileSizeEstimates_[pile] = pages;
	tileSizeEstimates_[0] += pages;

	if (tile->childCount == 0)
	{
		tileIndex_[tile->tip] = 0;
		fillGrid(tile->tile, pile);
	}
	else
	{
		assert(tile->maxChildCount);
		int step = Bits::countTrailingZerosInNonZero(tile->maxChildCount) / 2;
		assert((tile->maxChildCount == 4 && step == 1) ||
			(tile->maxChildCount == 16 && step == 2) ||
			(tile->maxChildCount == 64 && step == 3));

		tileIndex_[tile->tip] = 0;
		int childTip = tile->tip + ((step == 3) ? 3 : 2);
		uint64_t childTileMask = 0;
		for (int i = 0; i < tile->maxChildCount; i++)
		{
			STile* childTile = tile->children[i];
			if (childTile)
			{
				childTileMask |= (1ULL << i);
				tileIndex_[childTip] = ((childTile->tip - childTip) << 2) | 1;
					// For simplicity, we always write a branch tile pointer; 
					// if the child tile is a leaf tile, buildTile overwrites
					// this entry with 0
				buildTile(childTile);
				childTip++;
			}
			else
			{
				Tile childTile = Tile::fromColumnRowZoom(
					(tile->tile.column() << step) + (i & ((1 << step) - 1)),
					(tile->tile.row() << step) + (i >> step),
					tile->tile.zoom() + step);
				fillGrid(childTile, pile);
			}
		}
		tileIndex_[tile->tip + 1] = static_cast<uint32_t>(childTileMask);
		if (step == 3)
		{
			tileIndex_[tile->tip + 2] = static_cast<uint32_t>(childTileMask >> 32);
		}
	}
}


void TileIndexBuilder::fillGrid(Tile tile, int pile)
{
	//Console::msg("Filling grid for %s with %d", tile.toString().c_str(), pile);
	int step = maxZoom_ - tile.zoom();
	int extent = 1 << step;
	int left = tile.column() << step;
	int right = left + extent - 1;
	int top = tile.row() << step;
	int bottom = top + extent - 1;
	for (int row = top; row <= bottom; row++)
	{
		for (int col = left; col <= right; col++)
		{
			cellToPile_[cellOf(col, row)] = pile;
		}
	}
}

STile* TileIndexBuilder::Tier::Iterator::next() noexcept
{
	STile* tile = next_;
	if (tile) next_ = tile->next;
	return tile;
}