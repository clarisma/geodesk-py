// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "HilbertIndexBuilder.h"
#include <common/util/log.h>
#include "TFeature.h"
#include "TIndex.h"
#include "geom/rtree/hilbert.h"


TIndexTrunk* HilbertIndexBuilder::build(TFeature* firstFeature, int count)
{
	size_t workspaceSize = count * sizeof(HilbertItem);
	uint8_t* workspace = arena_.alloc(workspaceSize, 8);

	// Sort the features by their distance along the Hilbert Curve

	HilbertItem* hilbertItems = reinterpret_cast<HilbertItem*>(workspace);
	HilbertItem* p = hilbertItems;
	TFeature* feature = firstFeature;
	do
	{
		FeaturePtr f = feature->feature();
		if (f.isNode())
		{
			p->first = hilbert::calculateHilbertDistance(NodePtr(f).xy(), tileBounds_);
		}
		else
		{
			Box bounds = Box::simpleIntersection(f.bounds(), tileBounds_);
			assert(bounds.contains(bounds.center()));
			if (!tileBounds_.containsSimple(bounds))
			{
				LOG("%s not contained in tile bounds", f.toString().c_str());
			}
			p->first = hilbert::calculateHilbertDistance(bounds.center(), tileBounds_);
		}
		p->second = feature;
		p++;
		feature = feature->nextFeature();
	}
	while (feature != firstFeature);
	assert(p - hilbertItems == count);
	assert(reinterpret_cast<uint8_t*>(p) == workspace + workspaceSize);

	std::sort(hilbertItems, p);

	// Create the leaf branches of the spatial index

	TIndexBranch** branches = reinterpret_cast<TIndexBranch**>(workspace);
	TIndexBranch** pBranch = branches;
	p = hilbertItems;

	int parentCount = 0;
	for(;;)
	{
		parentCount++;
		if (count <= rtreeBucketSize_)
		{
			*pBranch = createLeaf(p, count);
			break;
		}
		*pBranch++ = createLeaf(p, rtreeBucketSize_);
		p += rtreeBucketSize_;
		count -= rtreeBucketSize_;
	}

	// Create the parent branches

	for (;;)
	{
		count = parentCount;
		pBranch = branches;
		parentCount = 0;
		TIndexBranch** pParentBranch = branches;
		for (;;)
		{
			parentCount++;
			if (count <= rtreeBucketSize_)
			{
				*pParentBranch = createTrunk(pBranch, count);
				break;
			}
			*pParentBranch++ = createTrunk(pBranch, rtreeBucketSize_);
			pBranch += rtreeBucketSize_;
			count -= rtreeBucketSize_;
		}
		if (parentCount == 1) break;
	}
	return *reinterpret_cast<TIndexTrunk**>(branches);
}


TIndexLeaf* HilbertIndexBuilder::createLeaf(HilbertItem* pChildren, int count)
{
	// LOG("Creating leaf (%d children at %016llX)...", count, pChildren);
	TFeature* firstFeature = nullptr;
	Box bounds;
	do
	{
		count--;
		TFeature* feature = pChildren[count].second;
		assert(feature->type() == TElement::Type::FEATURE);
		feature->setNext(firstFeature);
		firstFeature = feature;
		FeaturePtr f = feature->feature();
		if (f.isNode())
		{
			bounds.expandToInclude(NodePtr(f).xy());
		}
		else
		{
			bounds.expandToIncludeSimple(f.bounds());
		}
	}
	while (count);
	// TODO: Do we need to constrain the bbox to the tile bounds?
	return new(arena_.alloc<TIndexLeaf>()) TIndexLeaf(bounds, firstFeature);
}


TIndexTrunk* HilbertIndexBuilder::createTrunk(TIndexBranch** pChildBranches, int count)
{
	// LOG("Creating trunk (%d children at %016llX)...", count, pChildBranches);
	int originalCount = count;
	TIndexBranch* firstBranch = nullptr;
	Box bounds;
	do
	{
		count--;
		TIndexBranch* branch = pChildBranches[count];
		branch->setNextSibling(firstBranch);
		firstBranch = branch;
		bounds.expandToIncludeSimple(branch->bounds());
	}
	while (count);
	return new(arena_.alloc<TIndexTrunk>()) TIndexTrunk(bounds, firstBranch, originalCount);
}
