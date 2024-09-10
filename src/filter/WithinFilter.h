// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "feature/RelationPtr.h"
#include "geom/mc/MCIndexBuilder.h"
#include "PreparedSpatialFilter.h"
#include "PreparedFilterFactory.h"


class WithinPolygonFilter : public PreparedSpatialFilter
{
public:
	WithinPolygonFilter(const Box& bounds, MCIndex&& index) :
		PreparedSpatialFilter(bounds, std::move(index)) 
	{
		flags_ |= 
			FilterFlags::FAST_TILE_FILTER |
			FilterFlags::MUST_ACCEPT_ALL_MEMBERS | 
			FilterFlags::STRICT_BBOX;
	}

	WithinPolygonFilter(FeatureStore* store, RelationPtr areaRelation) :
		PreparedSpatialFilter(
			areaRelation.bounds(),
			MCIndexBuilder::buildFromAreaRelation(store, areaRelation))
	{
	}

	bool accept(FeatureStore* store, FeaturePtr feature, FastFilterHint fast) const override;
	int acceptTile(Tile tile) const override;
	
protected:
	bool acceptWay(WayPtr way) const override;
	bool acceptNode(NodePtr node) const override;
	bool acceptAreaRelation(FeatureStore* store, RelationPtr relation) const override;
	bool acceptMembers(FeatureStore* store, RelationPtr relation, RecursionGuard& guard) const override;
	int locateMembers(FeatureStore* store, RelationPtr relation, RecursionGuard& guard) const;

	int locateWayNodes(WayPtr way) const;
	bool containsWay(WayPtr way) const;
};


class WithinFilterFactory : public PreparedFilterFactory
{
public:
	const Filter* forPolygonal() override
	{
		return new WithinPolygonFilter(bounds(), buildIndex());
	}

	const Filter* forLineal() override
	{
		// return new WithinLinealFilter(bounds(), buildIndex());
		// TODO
		return nullptr;
	}

	const Filter* forCoordinate(Coordinate point) override
	{
		// TODO
		return nullptr;
	}
};
