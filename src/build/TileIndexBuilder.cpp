#include "TileIndexBuilder.h"
#include <unordered_map>

TileIndexBuilder::TileIndexBuilder(const BuildSettings& settings) :
	arena_(64 * 1024),
	settings_(settings),
	tierCount_(0)
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

uint32_t TileIndexBuilder::sumTileCounts() const noexcept
{
	uint32_t tileCount = 0;
	for (int i = 0; i < tierCount_; i++)
	{
		tileCount += tiers_[i].tileCount;
	}
	return tileCount;
}

const uint32_t* TileIndexBuilder::build(const uint32_t* nodeCounts)
{
	createLeafTiles(nodeCounts);
	addParentTiles();
	trimTiles();
	linkChildTiles();
	uint32_t indexSize = layoutIndex();
	uint32_t slotCount = indexSize / 4;
	uint32_t* pIndex = new uint32_t[slotCount];
	pIndex[0] = slotCount-1;
	tiers_[0].firstTile->write(pIndex);

	/*
	for (STile* t : tiles_)
	{
		printf("%s at TIP %u\n", t->tile().toString().c_str(), t->tip());
	}
	*/

	printf("- %u tiles\n", sumTileCounts());
	printf("- %u TIPs\n", pIndex[0]);
	return pIndex;
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

		tier = &tiers_[tierCount_];
		tier->level = leafLevel;
		tier->skippedLevels = 0;
	}
	tier->firstTile = tile;
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
	
	int startTier = tiers_[tierCount_].level < settings_.leafZoomLevel() ?
		tierCount_ : (tierCount_ - 1);
	startTier = std::max(startTier, 1);

	for (int i = startTier; i > 0; i--)
	{
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
				// printf("Created parent tile %s\n", parentTile.toString().c_str());
			}
			else
			{
				pt = it->second;
			}
			// We don't actually add the child to the parent yet,
			// we just mark the parent
			ct->setParent(pt);
			pt->addNodeCount(ct);
			if (ct->nodeCount() >= minTileDensity) childTier.addTile(ct);
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
		}
	}
}


uint32_t TileIndexBuilder::layoutIndex()
{
	uint32_t size = tiers_[0].firstTile->layout(4);		// First slot of table is unused
	printf("TileIndex size: %u bytes\n", size);
	return size;
}


TileIndexBuilder::STile::STile(Tile tile, uint32_t maxChildren, uint64_t nodeCount, STile* next) :
	next_(next),
	location_(0),
	tile_(tile),
	maxChildren_(maxChildren),
	childCount_(0),
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
	// printf("Writing tile %s\n", tile_.toString().c_str());
	assert(!isLeaf());
	assert(location_);
	assert(maxChildren_);
	int step = Bits::countTrailingZerosInNonZero(maxChildren_) / 2;
	assert((maxChildren_ == 4 && step == 1) || (maxChildren_ == 16 && step == 2) || (maxChildren_ == 64 && step == 2));
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
