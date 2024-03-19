#include "TileIndexBuilder.h"
#include <unordered_map>
#include <common/cli/Console.h>

TileIndexBuilder::TileIndexBuilder(const BuildSettings& settings) :
	arena_(64 * 1024),
	settings_(settings),
	tierCount_(0),
	tileCount_(0)
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
	//printf("  Created leaf tiles.\n");
	addParentTiles();
	//printf("  Added parent tiles.\n");
	trimTiles();
	linkChildTiles();
	//printf("  Linked child tiles.\n");
	uint32_t indexSize = layoutIndex();
	uint32_t slotCount = indexSize / 4;
	tileIndex_.reset(new uint32_t[slotCount]);
	tileIndex_[0] = slotCount-1;
	tiers_[0].firstTile->write(tileIndex_.get());

	calculateTileSizeEstimates();
	/*
	for (STile* t : tiles_)
	{
		printf("%s at TIP %u\n", t->tile().toString().c_str(), t->tip());
	}
	*/

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
				return a->nodeCount() > b->nodeCount();  // Use > for descending order
			});
		p = pTiles + settings_.maxTiles();
		do
		{
			(*p++)->clearNodeCount();
		}
		while (p < pEnd);
		tileCount_ = settings_.maxTiles();
	}
}

void TileIndexBuilder::createLeafTiles(const uint32_t* nodeCounts)
{
	int leafLevel = settings_.leafZoomLevel();
	int extent = 1 << leafLevel;
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
				Tile::fromColumnRowZoom(col, row, settings_.leafZoomLevel()),
				0, nodeCount, tile);
			count++;
		}
	}

	Tier* tier = &tiers_[tierCount_ - 1];
	if (tier->level < leafLevel)
	{
		// If the tile pyramid does not include zoom level 12,
		// we create a fake tier to store the level-12 tiles

		// TODO: In that case, the Analyzer should create a coarser grid instead

		tier = &tiers_[tierCount_];
		tier->level = leafLevel;
		tier->skippedLevels = 0;
	}
	tier->firstTile = tile;
	// printf("firstTile of zoom %d = %p\n", tier->level, tile);
	tier->tileCount = count;
}
	
TileIndexBuilder::STile* TileIndexBuilder::createTile(
	Tile tile, uint32_t maxChildCount, uint64_t nodeCount, STile* next)
{
	STile* pt = arena_.allocWithExplicitSize<STile>(sizeof(STile) +
		(static_cast<int32_t>(maxChildCount)-1) * sizeof(STile*));
	new(pt) STile(tile, maxChildCount, nodeCount, next);
	return pt;
}


void TileIndexBuilder::addParentTiles()
{
	std::unordered_map<Tile, STile*> parentTileMap;
	uint32_t minTileDensity = settings_.minTileDensity();
	
	//printf("Adding parent tiles...\n");
	//printf("tierCount_ = %d\n", tierCount_);
	int startTier = tiers_[tierCount_-1].level < settings_.leafZoomLevel() ?
		tierCount_ : (tierCount_ - 1);
	startTier = std::max(startTier, 1);
	//printf("startTier = %d\n", startTier);

	for (int i = startTier; i > 0; i--)
	{
		//printf("  Tier %d (firstTile = %p)\n", i, tiers_[i].firstTile);
		Tier& childTier = tiers_[i];
		Tier& parentTier = tiers_[i-1];
		int parentZoom = parentTier.level;
		uint32_t maxChildCount = parentTier.maxChildCount();
		// printf("Tier %d = zoom %d, %d max children\n", i, parentZoom, maxChildCount);

		Tier::Iterator iter = childTier.iter();
		childTier.clearTiles();
		STile* firstParentTile = nullptr;
		for (;;)
		{
			STile* ct = iter.next();
			if (!ct) break;

			Tile parentTile = ct->tile().zoomedOut(parentZoom);
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
			ct->setParent(pt);
			pt->addNodeCount(ct);
			pt->addEstimatedSize(ct->estimatedTileSize() /
				ESTIMATED_CHILD_BYTES_PER_PARENT_BYTE);
			if (ct->nodeCount() >= minTileDensity)
			{
				childTier.addTile(ct);
			}
			else
			{
				pt->addEstimatedSize(ct->estimatedTileSize());
			}
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
			STile* parent = tile->parent();
			assert(parent);
			// printf("Adding %s to %s...\n", tile->tile().toString().c_str(),
			//	parent->tile().toString().c_str());
			if (tile->nodeCount())
			{
				parent->addChild(tile);
				tier.addTile(tile);
			}
			else
			{
				uint64_t estimatedChildSize = tile->estimatedTileSize();
				parent->addEstimatedSize(estimatedChildSize +
					estimatedChildSize / ESTIMATED_CHILD_BYTES_PER_PARENT_BYTE);
			}
		}
	}
}


uint32_t TileIndexBuilder::layoutIndex()
{
	uint32_t size = tiers_[0].firstTile->layout(4);		// First slot of table is unused
	Console::msg("TileIndex size: %u bytes", size);
	return size;
}


TileIndexBuilder::STile::STile(Tile tile, uint32_t maxChildren, uint64_t nodeCount, STile* next) :
	next_(next),
	location_(0),
	tile_(tile),
	maxChildren_(maxChildren),
	childCount_(0),
	estimatedTileSize_(nodeCount * ESTIMATED_BYTES_PER_NODE),
	totalNodeCount_(nodeCount),
	// ownNodeCount_(0),
	parent_(nullptr)
{
}

uint32_t TileIndexBuilder::STile::layout(uint32_t pos) noexcept
{
	// printf("Placing %s at %u...\n", tile_.toString().c_str(), pos);
	location_ = pos;
	pos += size();
	uint32_t childPos = pos - childCount_ * 4;
	for (int i = 0; i < childCount_; i++)
	{
		if (!children_[i]->isLeaf())
		{
			pos = children_[i]->layout(pos);
		}
		else
		{
			children_[i]->location_ = childPos;
		}
		childPos += 4;
	}
	return pos;
}

void TileIndexBuilder::STile::write(uint32_t* pIndex) noexcept
{
	/*
	printf("Writing tile %s (%d children) at tip %d\n", tile_.toString().c_str(), 
		childCount_, location_ / 4);
	*/
	assert(!isLeaf() || tile_ == Tile::fromColumnRowZoom(0,0,0));
	assert(location_);
	assert(maxChildren_);
	int step = Bits::countTrailingZerosInNonZero(maxChildren_) / 2;
	assert((maxChildren_ == 4 && step == 1) || (maxChildren_ == 16 && step == 2) || (maxChildren_ == 64 && step == 3));
	uint64_t childTileMask = 0;
	int left = tile_.column() << step;
	int top = tile_.row() << step;
	int width = 1 << step;
	for (int i = 0; i < childCount_; i++)
	{
		Tile childTile = children_[i]->tile();
		int num = (childTile.row() - top) * width + (childTile.column() - left);
		assert(num >= 0 && num < maxChildren_);
		childTileMask |= 1ULL << num;
	}
	uint32_t* p = &pIndex[location_ / 4];
	uint32_t* pStart = p;
	*p++ = 0;
	*p++ = static_cast<uint32_t>(childTileMask);
	if (step == 3)
	{
		assert(maxChildren_ == 64);
		*p++ = static_cast<uint32_t>(childTileMask >> 32);
	}
	for (int i = 0; i < childCount_; i++)
	{
		if (children_[i]->isLeaf())
		{
			*p++ = 0;
		}
		else
		{
			*p++ = (children_[i]->location_ - (location_ +
				(p - pStart) * sizeof(uint32_t))) | 1;
			children_[i]->write(pIndex);
		}
	}
}


void TileIndexBuilder::calculateTileSizeEstimates()
{
	tileSizeEstimates_.reset(new uint32_t[tileCount_ + 1]);
	tileSizeEstimates_[0] = 0;
	int nextPile = tileSizeEstimate(1, tiers_[0].firstTile);
	assert(nextPile - 1 == tileCount_);
}

int TileIndexBuilder::tileSizeEstimate(int pile, STile* tile)
{
	uint32_t pageSize = settings_.featurePilesPageSize();
	uint32_t pages = (tile->estimatedTileSize() + pageSize - 1) / pageSize;
	tileSizeEstimates_[pile] = pages;
	tileSizeEstimates_[0] += pages;
	pile++;
	for (int i = 0; i < tile->childCount_; i++)
	{
		pile = tileSizeEstimate(pile, tile->children_[i]);
	}
	return pile;
}

