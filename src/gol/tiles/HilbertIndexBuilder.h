#pragma once
#include <utility> // for std::pair
#include "TTile.h"

class TFeature;
class TIndexLeaf;

class HilbertIndexBuilder
{
public:
	HilbertIndexBuilder(TTile& tile) :
		arena_(tile.arena()),
		tileBounds_(tile.bounds()),
		rtreeBucketSize_(9)             // TODO
	{
	}

	TIndexTrunk* build(TFeature* firstFeature, int count);

private:
	using HilbertItem = std::pair<uint32_t, TFeature*>;

	TIndexLeaf* createLeaf(HilbertItem* pFirst, int count);
	TIndexTrunk* createTrunk(TIndexBranch** pFirst, int count);

	Arena& arena_;
	Box tileBounds_;
	const int rtreeBucketSize_;
};
