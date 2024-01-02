// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <geos_c.h>
#include "MonotoneChain.h"
#include "MCIndex.h"
#include "feature/Way.h"
#include "feature/Relation.h"
#include <common/alloc/Arena.h>

class MCIndexBuilder
{
public:
	MCIndexBuilder();
	void addLineSegment(Coordinate start, Coordinate end);
	void segmentizeWay(WayRef way);
	void segmentizeCoords(GEOSContextHandle_t context, const GEOSCoordSequence* coords);
	void segmentizePolygon(GEOSContextHandle_t context, const GEOSGeometry* polygon);
	void segmentizeAreaRelation(FeatureStore* store, RelationRef rel);
	void segmentizeMembers(FeatureStore* store, RelationRef rel, RecursionGuard& guard);
	MCIndex build(Box bounds);
	static MCIndex buildFromAreaRelation(FeatureStore* store, RelationRef rel)
	{
		MCIndexBuilder builder;
		builder.segmentizeAreaRelation(store, rel);
		return builder.build(rel.bounds());
	}

private:
	// static const size_t CHUNK_SIZE = 32 * 1024;
	static const int MAX_VERTEX_COUNT = 256;

	class MCHolder 
	{
	public:
		static size_t storageSize(int vertexCount)
		{
			return sizeof(MCHolder) - sizeof(MonotoneChain) + 
				MonotoneChain::storageSize(vertexCount);
		}

		const MCHolder* next;
		uint32_t padding;     // explicit padding (MC always has an odd number of ints)
		MonotoneChain chain;
	};

	size_t chainCount_;
	size_t totalChainSize_;
	const MCHolder* first_;
	Arena arena_;
};
