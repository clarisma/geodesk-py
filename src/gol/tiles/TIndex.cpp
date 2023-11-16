#include "TIndex.h"
#include "TTile.h"
#include "HilbertIndexBuilder.h"
#include "IndexSettings.h"

uint32_t TIndexLeaf::calculateSize(TFeature* firstFeature)
{
	uint32_t size = 0;
	TFeature* p = firstFeature;
	do
	{
		size += p->size();
		p = p->next();
	}
	while (p);
	return size;
}


void TIndexLeaf::write(const TTile* tile, uint8_t* p) const
{
	TFeature* feature = firstFeature();
	do
	{
		feature->write(tile, p);
		p += feature->size();
		feature = feature->next();
	}
	while (feature);
}


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

void TIndexTrunk::write(uint8_t* pStart) const
{
	int32_t pos = location();
	uint8_t* p = pStart;
	TIndexBranch* child = firstChildBranch();
	do
	{
		TIndexBranch* nextChild = child->next();
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
	TElement(0, 0, TElement::Alignment::DWORD),
	firstRoot_(-1),
	lastRoot_(-1),
	rootCount_(0)
{
	memset(&roots_, 0, sizeof(roots_));
	memset(&next_, -1, sizeof(next_));
}

void TIndex::Root::addFeature(TFeature* feature, uint32_t indexBits)
{
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
	assert (!isEmpty());
	assert (!other.isEmpty());
	indexBits |= other.indexBits;
	featureCount += other.featureCount;
	TFeature* next = firstFeature->next();
	firstFeature->setNext(other.firstFeature->next());
	other.firstFeature->setNext(next);
	other.featureCount = 0;
	other.firstFeature = nullptr;
}


void TIndex::Root::build(HilbertIndexBuilder& rtreeBuilder)
{
	trunk = rtreeBuilder.build(firstFeature, featureCount);
}

void TIndex::addFeature(TFeature* feature, const IndexSettings& settings)
{
	// TODO: get category, get index bits
	int category = 0;	// TODO
	uint32_t indexBits = 0; // TODO
	roots_[category].addFeature(feature, indexBits);
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
			int8_t* pNextRoot = &firstRoot_;
			for (;;)
			{
				int nextRoot = *pNextRoot;
				if (nextRoot == -1)
				{
					lastRoot_ = static_cast<int8_t>(i);
				}
				else if (roots_[nextRoot].featureCount > root.featureCount)
				{
					pNextRoot = &next_[nextRoot];
					continue;
				}
				*pNextRoot = static_cast<int8_t>(i);
				next_[i] = -1;
				break;
			}
			rootCount_++;
		}
	}

	// The multi-cat root is always included, unless it has no features
	// and we're not over the root-count limit
	if (roots_[MULTI_CATEGORY].featureCount || rootCount_ > maxRootCount)
	{
		next_[lastRoot_] = lastRoot_ = MULTI_CATEGORY;
		rootCount_++;
	}

	// Build the rtree for all roots that are below maxRootCount
	// (except for multi-cat root)

	int8_t* pNextRoot = &firstRoot_;
	int i = 0;
	for (; i < maxRootCount-1; i++)
	{
		int nextRoot = *pNextRoot;
		roots_[nextRoot].build(rtreeBuilder);
		pNextRoot = &next_[nextRoot];
	}

	// If there are more roots than maxRootCount, consolidate the
	// roots with the lowest numbers of features into the multi-cat root

	for (; i < rootCount_ - 1; i++)
	{
		int nextRoot = *pNextRoot;
		*pNextRoot = MULTI_CATEGORY;
		roots_[MULTI_CATEGORY].add(roots_[nextRoot]);
		pNextRoot = &next_[nextRoot];
	}

	// Adjust the root count
	rootCount_ = std::min(rootCount_, maxRootCount);

	// finally, build the rtree for the multi-cat root
	roots_[MULTI_CATEGORY].build(rtreeBuilder);
}


void TIndex::write(uint8_t* p) const
{
	uint8_t pos = location();
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
	for (;;)
	{
		TFeature* feature = iter.next();
		if (!feature) break;
		int typeFlags = (feature->flags() >> 1) & 15;
		int type = FLAGS_TO_TYPE[typeFlags];
		assert(type != INVALID);		// TODO: make this a proper runtime check?
		indexes_[type].addFeature(feature, settings_);
	}
}


void Indexer::build()
{
	for (int i = 0; i < 4; i++)
	{
		indexes_[i].build(tile_, settings_);
	}
}
