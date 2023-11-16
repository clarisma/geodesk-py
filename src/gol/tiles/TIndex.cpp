#include "TIndex.h"
#include "TTile.h"

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
	firstRoot_(0)
{
	memset(&roots_, 0, sizeof(roots_));
	memset(&next_, -1, sizeof(next_));
}

void TIndex::addFeature(TFeature* feature)
{
	Root& root = roots_[getFeatureCategory(feature)];
	feature->setNext(root.firstFeature);
	root.firstFeature = feature;
	root.count++;
}

void TIndex::build()
{
	// TODO
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
		indexes_[type].addFeature(feature);
	}
}
