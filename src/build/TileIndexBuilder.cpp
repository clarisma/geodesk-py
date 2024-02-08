#include "TileIndexBuilder.h"
#include <unordered_map>

TileIndexBuilder::TileIndexBuilder(const BuildSettings& settings) :
	arena_(64 * 1024),
	root_(nullptr),
	settings_(settings),
	tierCount_(0)
{
	tiles_.reserve(settings.maxTiles());	// TODO: allocate more?

	ZoomLevels levels = settings.zoomLevels();
	ZoomLevels::Iterator iter = levels.iter();
	for (;;)
	{
		int zoom = iter.next();
		if (zoom < 0) break;
		tierLevels_[tierCount_] = zoom;
		tierSkippedLevelCounts_[tierCount_] = levels.skippedAfterLevel(zoom);
		tierCount_++;
	}
}

const uint32_t* TileIndexBuilder::build(const uint32_t* nodeCounts)
{
	createLeafTiles(nodeCounts);
	addParentTiles();
	if (tiles_.size() > settings_.maxTiles())
	{
		// If there are too many tiles, keep only the largest

		std::sort(tiles_.begin(), tiles_.end(), [](const STile* a, const STile* b)
			{
				return a->count() > b->count();  // Use > for descending order
			});
		tiles_.erase(tiles_.begin() + settings_.maxTiles());
	}
	linkChildTiles();
	uint32_t indexSize = layoutIndex();
	uint32_t slotCount = indexSize / 4;
	uint32_t* pIndex = new uint32_t[slotCount];
	pIndex[0] = slotCount-1;
	root_->write(pIndex);

	for (STile* t : tiles_)
	{
		printf("%s at TIP %u\n", t->tile().toString().c_str(), t->tip());
	}

	printf("- %zu tiles\n", tiles_.size());
	printf("- %u TIPs\n", pIndex[0]);
	return pIndex;
}

void TileIndexBuilder::createLeafTiles(const uint32_t* nodeCounts)
{
	int extent = 1 << settings_.leafZoomLevel();
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
			tiles_.push_back(createTile(
				Tile::fromColumnRowZoom(col, row, settings_.leafZoomLevel()),
				0, nodeCount));
		}
	}
}
	
TileIndexBuilder::STile* TileIndexBuilder::createTile(
	Tile tile, uint32_t maxChildCount, uint64_t nodeCount)
{
	STile* pt = arena_.allocWithExplicitSize<STile>(sizeof(STile) +
		(static_cast<int32_t>(maxChildCount)-1) * sizeof(STile*));
	new(pt) STile(tile, maxChildCount, nodeCount);
	return pt;
}


void TileIndexBuilder::addParentTiles()
{
	std::unordered_map<Tile, STile*> parentTiles;
	std::vector<STile*> childTiles;
	std::vector<STile*>* pChildTiles = &tiles_;

	for (int tier = std::max(tierCount_-2, 0U); tier >= 0; tier--)
	{
		int parentZoom = tierLevels_[tier];
		uint32_t maxChildCount = maxChildCountOfTier(tier);
		printf("Tier %d = zoom %d, %d max children", tier, parentZoom, maxChildCount);

		for (STile* ct : *pChildTiles)
		{
			Tile parentTile = ct->tile().zoomedOut(parentZoom);
			STile* pt;
			auto it = parentTiles.find(parentTile);
			if (it == parentTiles.end())
			{
				pt = createTile(parentTile, maxChildCount, 0);
				parentTiles[parentTile] = pt;
				// printf("Created parent tile %s\n", parentTile.toString().c_str());
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
			if (pt->count() >= settings_.minTileDensity())
			{
				// Add parent tile to the main tile collection
				// only if it meets the minimum node count
				tiles_.push_back(pt);
			}
		}
		parentTiles.clear();
		pChildTiles = &childTiles;
	}
	assert(childTiles.size() == 1);
	root_ = childTiles[0];
}


void TileIndexBuilder::linkChildTiles()
{
	for (STile* tile : tiles_)
	{
		if (tile != root_)
		{
			STile* parent = tile->parent();
			assert(parent);
			// printf("Adding %s to %s...\n", tile->tile().toString().c_str(),
			//	parent->tile().toString().c_str());
			parent->addChild(tile);
		}
	}
}


uint32_t TileIndexBuilder::layoutIndex()
{
	uint32_t size = root_->layout(4);		// First slot of table is unused
	printf("TileIndex size: %u bytes\n", size);
	return size;
}


uint32_t TileIndexBuilder::STile::layout(uint32_t pos) noexcept
{
	printf("Placing %s at %u...\n", tile_.toString().c_str(), pos);
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
