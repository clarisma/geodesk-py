#include "TIndex.h"
#include <common/util/log.h>
#include "Layout.h"
#include "HilbertIndexBuilder.h"
#include "IndexSettings.h"
#include "TTagTable.h"
#include "TTile.h"

uint32_t TIndexLeaf::calculateSize(TFeature* firstFeature)
{
	uint32_t size = 0;
	TFeature* p = firstFeature;
	do
	{
		size += p->size();
		p = p->nextFeature();
	}
	while (p);
	return size;
}

/*
void TIndexLeaf::write(const TTile* tile, uint8_t* p) const
{
	TFeature* feature = firstFeature();
	do
	{
		feature->write(tile, p);
		p += feature->size();
		feature = feature->nextFeature();
	}
	while (!feature->isLast());
}
*/

/*
uint32_t TIndexTrunk::calculateSize(TIndexBranch* firstBranch)
{
	uint32_t size = 0;
	TIndexBranch* p = firstBranch;
	do
	{
		size += 20;			// 16 bytes for bounds, 4 bytes for pointer
		p = p->next();
	}
	while (p);
	return size;
}
*/

void TIndexTrunk::write(const TTile& tile) const
{
	uint8_t pos = location();
	uint8_t* p = tile.newTileData() + pos;
	TIndexBranch* child = firstChildBranch();
	do
	{
		TIndexBranch* nextChild = child->nextSibling();
		*reinterpret_cast<int32_t*>(p) = (child->location() - pos)
			| (nextChild ? 0 : 1)			// last_item flag
			| (child->isLeaf() ? 2 : 0);	// is_leaf flag
		*reinterpret_cast<Box*>(p+4) = child->bounds();
		p += 20;
		pos += 20;
			// TODO: Can we safely assume same memory layout of Box? 
		child = nextChild;
	}
	while (child);
}


TIndex::TIndex() :
	TElement(Type::INDEX, 0, 0, TElement::Alignment::DWORD),
	firstRoot_(-1),
	rootCount_(0)
{
	memset(&roots_, 0, sizeof(roots_));
	memset(&next_, -1, sizeof(next_));
}

void TIndex::Root::addFeature(TFeature* feature, uint32_t indexBits)
{
	assert((featureCount == 0) == (firstFeature == nullptr));
	if (firstFeature)
	{
		feature->setNext(firstFeature->next());
		firstFeature->setNext(feature);
	}
	else
	{
		firstFeature = feature;
		feature->setNext(feature);
	}
	featureCount++;
	this->indexBits |= indexBits;
}

void TIndex::Root::add(Root& other)
{
	assert((featureCount == 0) == (firstFeature == nullptr));
	assert(&other != this);
	if (other.isEmpty()) return;
	indexBits |= other.indexBits;
	if (isEmpty())
	{
		firstFeature = other.firstFeature;
	}
	else
	{
		TFeature* next = firstFeature->nextFeature();
		firstFeature->setNext(other.firstFeature->nextFeature());
		other.firstFeature->setNext(next);
	}
	featureCount += other.featureCount;
	other.featureCount = 0;
	other.firstFeature = nullptr;
}


void TIndex::Root::build(HilbertIndexBuilder& rtreeBuilder)
{
	trunk = rtreeBuilder.build(firstFeature, featureCount);
}


void TIndex::build(TTile& tile, const IndexSettings& settings)
{
	int maxRootCount = settings.maxKeyIndexes();
	int minFeaturesPerRoot = settings.keyIndexMinFeatures();
	int rtreeBranchSize = settings.rtreeBucketSize();

	HilbertIndexBuilder rtreeBuilder(tile);

	// - Sort roots by number of features
	// - Consolidate Roots with less features than minFeaturesPerRoot 
	//   into multi-cat root.
	
	for (int i = 0; i < MULTI_CATEGORY; i++)
	{
		Root& root = roots_[i];
		if (root.featureCount < minFeaturesPerRoot)
		{
			// Consolidate this root into the multi-cat root
			roots_[MULTI_CATEGORY].add(root);
		}
		else
		{
			// Otherwise, place this root into a linked list, 
			// largest roots first

			int8_t* pNextRoot = &firstRoot_;
			for (;;)
			{
				int nextRoot = *pNextRoot;
				if (nextRoot >= 0 && roots_[nextRoot].featureCount > root.featureCount)
				{
					pNextRoot = &next_[nextRoot];
					continue;
				}
				*pNextRoot = static_cast<int8_t>(i);
				next_[i] = nextRoot;
				break;
			}
			rootCount_++;
		}
	}

	// rootCount_ is now the total number of non-empty roots, 
	// excluding the multi-cat root

	// include multi-cat if it has any features
	int rootCountWithMultiCat = rootCount_ + (roots_[MULTI_CATEGORY].isEmpty() ? 0 : 1);

	// The number of roots (excluding multi-cat) to keep as-is
	int keepRootCount = std::min(rootCount_, maxRootCount - 1);
	// (If we have 4 roots and multi-cat is empty, and the limit is 4,
	// this simply means the smallest root turns into the multi-cat
	// root, which simply consists of a single category)

	// Build the rtree for all roots that are below maxRootCount
	// (except for multi-cat root)

	int8_t* pNextRoot = &firstRoot_;
	for (int i = 0; i < keepRootCount; i++)
	{
		int nextRoot = *pNextRoot;
		assert (nextRoot >= 0);
		roots_[nextRoot].build(rtreeBuilder);
		pNextRoot = &next_[nextRoot];
	}

	// If there are more roots than maxRootCount, consolidate the
	// roots with the lowest numbers of features into the multi-cat root

	int8_t* pLastRoot = pNextRoot;

	int consolidateRootCount = rootCount_ - keepRootCount;
	for (int i=0; i < consolidateRootCount; i++)
	{
		int nextRoot = *pNextRoot;
		assert(nextRoot >= 0);
		roots_[MULTI_CATEGORY].add(roots_[nextRoot]);
		pNextRoot = &next_[nextRoot];
	}

	*pLastRoot = -1;

	// Adjust the root count
	rootCount_ = keepRootCount;

	if (!roots_[MULTI_CATEGORY].isEmpty())
	{
		// If there are any features in the multi-cat root,
		// add it to the end and build its rtree 
		*pLastRoot = MULTI_CATEGORY;
		roots_[MULTI_CATEGORY].build(rtreeBuilder);
		rootCount_++;
	}

	setSize(rootCount_ * 8);
}


void TIndex::place(Layout& layout)
{
	if (rootCount_ == 0) return;

	layout.place(this);
	int rootNumber = firstRoot_;
	do
	{
		roots_[rootNumber].trunk->place(layout);
		rootNumber = next_[rootNumber];
	}
	while (rootNumber >= 0);
}


/**
 * Place the feature in this leaf branch, then place any uncommon tag tables
 * that haven't already been placed.
 */
void TIndexLeaf::place(Layout& layout)
{
	LinkedQueue<TTagTable> tagTables;

	// Note: We don't place the index leaf itself; instead, we place the 
	// feature stubs that are contained in this leaf

	TFeature* feature = firstFeature();
	for(;;)
	{
		TFeature* nextFeature = feature->nextFeature();
		layout.place(feature);
		TTagTable* tags = feature->tags(layout.tile());
		assert(tags);
		if (tags->location() <= 0)
		{
			tagTables.addTail(tags);
		}
		if (!nextFeature)
		{
			feature->setLast(true);
			break;
		}
		tags->addStrings(layout);
		switch (feature->feature().typeCode())
		{
		case 0:
			reinterpret_cast<TNode*>(feature)->placeBody(layout);
			break;
		case 1:
			reinterpret_cast<TWay*>(feature)->placeBody(layout);
			break;
		case 2:
			reinterpret_cast<TRelation*>(feature)->placeBody(layout);
			break;
		default:
			assert(false);
			break;
		}
		feature = nextFeature;
	}
	 
	TTagTable* tags = tagTables.first();
	while (tags)
	{
		TTagTable* nextTags = tags->nextTags();
		layout.place(tags);		// place() changes `next`
		tags = nextTags;
	}
}



void TIndexTrunk::place(Layout& layout)
{
	layout.place(this);
	TIndexBranch* branch = firstChildBranch();
	do
	{
		if (branch->isLeaf())
		{
			reinterpret_cast<TIndexLeaf*>(branch)->place(layout);
		}
		else
		{
			reinterpret_cast<TIndexTrunk*>(branch)->place(layout);
		}
		branch = branch->nextSibling();
	}
	while (branch);
}


void TIndex::write(const TTile& tile) const
{
	uint8_t pos = location();
	uint8_t* p = tile.newTileData() + pos;
	int rootNumber = firstRoot_;
	do
	{
		int nextRootNumber = next_[rootNumber];
		const Root& root = roots_[rootNumber];
		*reinterpret_cast<int32_t*>(p) =
			(root.trunk->location() - pos) |
			(nextRootNumber < 0 ? 1 : 0);
		*reinterpret_cast<uint32_t*>(p + 4) = root.indexBits;
		p += 8;
		pos += 8;
		rootNumber = nextRootNumber;
	}
	while (rootNumber >= 0);
}

/**
 * If we right-shift the feature flags by 1, then take the bottom 4 bits,
 * we can tell to which index the feature belongs without having to branch
 * (We're interested in the type bits and the area-flag; we'll ignore the
 *  member flag)
 */
const uint8_t Indexer::FLAGS_TO_TYPE[16] =
{
	NODES, INVALID, NODES, INVALID,
	WAYS, AREAS, WAYS, AREAS,
	RELATIONS, AREAS, RELATIONS, AREAS,
	INVALID, INVALID, INVALID, INVALID
};


Indexer::Indexer(TTile& tile, const IndexSettings& settings) :
	tile_(tile),
	settings_(settings)
{
}


void Indexer::addFeatures(const FeatureTable& features)
{
	FeatureTable::Iterator iter = features.iter();
	while(iter.hasNext())
	{
		TFeature* feature = iter.next();

		// LOG("Indexing %s...", feature->feature().toString().c_str());
		int typeFlags = (feature->flags() >> 1) & 15;
		int type = FLAGS_TO_TYPE[typeFlags];
		assert(type != INVALID);		// TODO: make this a proper runtime check?
		TTagTable* tags = feature->tags(tile_);
		/*
		if (!tags)
		{
			if (!feature->feature().tags().hasLocalKeys())
			{
				LOG("  No tags (no locals)");
			}
			continue;
		}
		*/
		assert(tags);
		/*
		if (feature->feature().tags().hasLocalKeys())
		{
			LOG("  Tags (locals)");
		}
		*/
		int category = tags->category();
		uint32_t indexBits;
		if (category >= TIndex::MULTI_CATEGORY)
		{
			// Category is unassigned or multi-category
			// In both cases, we need to construct indexBits
			indexBits = tags->assignIndexCategory(settings_);
			category = tags->category();
		}
		else
		{
			indexBits = 1 << (category - 1);
		}
		indexes_[type].addFeature(feature, category, indexBits);
	}
}


void Indexer::build()
{
	for (int i = 0; i < 4; i++)
	{
		indexes_[i].build(tile_, settings_);
	}
}

void Indexer::place(Layout& layout)
{
	for (int i = 0; i < 4; i++)
	{
		indexes_[i].place(layout);
	}
}


