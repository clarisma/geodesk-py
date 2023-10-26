// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "feature/Relation.h"
#include "geom/mc/MCIndexBuilder.h"
#include "PreparedSpatialFilter.h"
#include "PreparedFilterFactory.h"


class WithinPolygonFilter : public PreparedSpatialFilter
{
public:
	WithinPolygonFilter(const Box& bounds, MCIndex&& index) :
		PreparedSpatialFilter(bounds, std::move(index)) 
	{
		flags_ |= FilterFlags::MUST_ACCEPT_ALL_MEMBERS | 
			FilterFlags::STRICT_BBOX;
	}

	WithinPolygonFilter(FeatureStore* store, RelationRef areaRelation) :
		PreparedSpatialFilter(
			areaRelation.bounds(),
			MCIndexBuilder::buildFromAreaRelation(store, areaRelation))
	{
	}

	bool accept(FeatureStore* store, FeatureRef feature, FastFilterHint fast) const override;

protected:
	bool acceptWay(WayRef way) const override;
	bool acceptNode(NodeRef node) const override;
	bool acceptAreaRelation(FeatureStore* store, RelationRef relation) const override;
	bool acceptMembers(FeatureStore* store, RelationRef relation, RecursionGuard& guard) const override;
	int locateMembers(FeatureStore* store, RelationRef relation, RecursionGuard& guard) const;

	int locateWayNodes(WayRef way) const;
	bool containsWay(WayRef way) const;
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
