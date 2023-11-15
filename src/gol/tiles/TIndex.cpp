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