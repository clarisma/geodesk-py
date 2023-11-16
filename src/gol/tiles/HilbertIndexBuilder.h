#pragma once
#include <utility> // for std::pair
#include "TTile.h"

class TFeature;
class TIndexBranch;
class TIndexLeaf;
class TIndexTrunk;

class HilbertIndexBuilder
{
public:
	HilbertIndexBuilder(TTile& tile) :
		arena_(tile.arena()),
		tileBounds_(tile.bounds()),
		rtreeBucketSize_(9)             // TODO
	{
	}

	/**
	 * Builds a spatial index for a set of features. Note that
	 * features are in a CIRCULAR LIST, and an explicit count
	 * must be passed (which must match the number of features)
	 * 
	 * @param firstFeature a circular list of features
	 * @param the number of features
	 */
	TIndexTrunk* build(TFeature* firstFeature, int count);

private:
	using HilbertItem = std::pair<uint32_t, TFeature*>;

	TIndexLeaf* createLeaf(HilbertItem* pFirst, int count);
	TIndexTrunk* createTrunk(TIndexBranch** pFirst, int count);

	Arena& arena_;
	Box tileBounds_;
	const int rtreeBucketSize_;
};
