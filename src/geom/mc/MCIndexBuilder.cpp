// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "MCIndexBuilder.h"
#include "WaySlicer.h"
#include "CoordSequenceSlicer.h"
#include "feature/FeatureStore.h"
#include "feature/MemberIterator.h"
#include "feature/FastMemberIterator.h"
#include "geom/rtree/HilbertTreeBuilder.h"

MCIndexBuilder::MCIndexBuilder() :
	chainCount_(0),
	totalChainSize_(0),
	first_(nullptr),
	arena_(16 * 1024, Arena::GrowthPolicy::DOUBLE)
{
}


void MCIndexBuilder::segmentizeWay(WayRef way)
{
	WaySlicer slicer(way);
	do
	{
		MCHolder* holder = arena_.allocWithExplicitSize<MCHolder>(
			MCHolder::storageSize(MAX_VERTEX_COUNT));
		slicer.slice(&holder->chain, MAX_VERTEX_COUNT);
		// Give back the unused space to the Arena
		int unusedVertexes = MAX_VERTEX_COUNT - holder->chain.vertexCount();
		arena_.reduceLastAlloc(unusedVertexes * sizeof(Coordinate));
		holder->next = first_;
		first_ = holder;
		chainCount_++;
		totalChainSize_ += holder->chain.storageSize();
	}
	while (slicer.hasMore());
}

void MCIndexBuilder::segmentizeAreaRelation(FeatureStore* store, RelationRef rel)
{
	FastMemberIterator iter(store, rel);
	for (;;)
	{
		FeatureRef member = iter.next();
		if (member.isNull()) break;
		if (member.isWay())
		{
			WayRef way(member);
			if(!way.isPlaceholder()) segmentizeWay(way);
		}
	}

	// If no ways were extracted, attempt to extract any features
	// (i.e. treat like non-area relation)

	if (chainCount_ == 0)
	{
		RecursionGuard guard(rel);
		segmentizeMembers(store, rel, guard);
	}

	// TODO: We still need to ensure there is at least one chain;
	// chainCount_ could be 0 if relation only consists of nodes
}


void MCIndexBuilder::segmentizeMembers(FeatureStore* store, RelationRef rel, RecursionGuard& guard)
{
	FastMemberIterator iter(store, rel);
	for (;;)
	{
		FeatureRef member = iter.next();
		if (member.isNull()) break;
		int memberType = member.typeCode();
		if (memberType == 1)
		{
			WayRef memberWay(member);
			if (memberWay.isPlaceholder()) continue;
			segmentizeWay(memberWay);
		}
		else if(memberType == 2)
		{
			RelationRef childRel(member);
			if (childRel.isPlaceholder() || !guard.checkAndAdd(childRel)) continue;
			segmentizeMembers(store, childRel, guard);
		}
	}
}

// TODO: must be able to deal with empty areas (i.e. chainCount_ == 0)
MCIndex MCIndexBuilder::build(Box bounds)
{
	assert(chainCount_ > 0);
	assert(totalChainSize_ > 0);
	uint8_t* data = new uint8_t[totalChainSize_];
	BoundedItem* boundedItems = arena_.allocArray<BoundedItem>(chainCount_);
	const MCHolder* holder = first_;
	BoundedItem* p = boundedItems;
	uint8_t* pNextNormalizedChain = data;
	while (holder)
	{
		MonotoneChain* pNormalizedChain = reinterpret_cast<MonotoneChain*>(pNextNormalizedChain);
		holder->chain.copyNormalized(pNormalizedChain);
		pNextNormalizedChain += pNormalizedChain->storageSize();
		p->item = pNormalizedChain;
		p->bounds = pNormalizedChain->bounds();
		p++;
		holder = holder->next;
	}
	assert(pNextNormalizedChain == data + totalChainSize_);
	assert(p == boundedItems + chainCount_);

	HilbertTreeBuilder indexBuilder(&arena_);
	return MCIndex(data, indexBuilder.build<const MonotoneChain>(
		boundedItems, chainCount_, 9, bounds));
}


void MCIndexBuilder::segmentizeCoords(GEOSContextHandle_t context, const GEOSCoordSequence* coords)
{
	CoordSequenceSlicer slicer(context, coords);
	do
	{
		MCHolder* holder = arena_.allocWithExplicitSize<MCHolder>(
			MCHolder::storageSize(MAX_VERTEX_COUNT));
		slicer.slice(&holder->chain, MAX_VERTEX_COUNT);
		// Give back the unused space to the Arena
		int unusedVertexes = MAX_VERTEX_COUNT - holder->chain.vertexCount();
		arena_.reduceLastAlloc(unusedVertexes * sizeof(Coordinate));
		holder->next = first_;
		first_ = holder;
		chainCount_++;
		totalChainSize_ += holder->chain.storageSize();
	}
	while (slicer.hasMore());
}


void MCIndexBuilder::segmentizePolygon(GEOSContextHandle_t context, const GEOSGeometry* polygon)
{
	// TODO: empty polygons
	const GEOSGeometry* ring = GEOSGetExteriorRing_r(context, polygon);
	if (ring == NULL) return;
	const GEOSCoordSequence* seq = GEOSGeom_getCoordSeq_r(context, ring);
	segmentizeCoords(context, seq);
	int holeCount = GEOSGetNumInteriorRings_r(context, polygon);
	for (int i = 0; i < holeCount; ++i) 
	{
		ring = GEOSGetInteriorRingN_r(context, polygon, i);
		if (ring == NULL) continue;
		seq = GEOSGeom_getCoordSeq_r(context, ring);
		segmentizeCoords(context, seq);
	}
}


void MCIndexBuilder::addLineSegment(Coordinate start, Coordinate end)
{
	MCHolder* holder = arena_.allocWithExplicitSize<MCHolder>(
		MCHolder::storageSize(2));
	holder->chain.initLineSegment(start, end);
	holder->next = first_;
	first_ = holder;
	chainCount_++;
	totalChainSize_ += holder->chain.storageSize();
}
