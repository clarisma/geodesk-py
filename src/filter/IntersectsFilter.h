// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "feature/RelationPtr.h"
#include "geom/mc/MCIndexBuilder.h"
#include "ContainsPointFilter.h"
#include "PreparedSpatialFilter.h"
#include "PreparedFilterFactory.h"


// TODO: Generalize PreparedSpatialFilter, have it take NodeRef, WayRef, RelationRef
//  (or just FeatureRef), GeosGeometry, etc.

class IntersectsPolygonFilter : public PreparedSpatialFilter
{
public:
	IntersectsPolygonFilter(const Box& bounds, MCIndex&& index) :
		PreparedSpatialFilter(bounds, std::move(index))
	{
		flags_ |= FilterFlags::FAST_TILE_FILTER;
	}

	bool accept(FeatureStore* store, FeaturePtr feature, FastFilterHint fast) const override;
	int acceptTile(Tile tile) const override;

protected:
	bool acceptWay(WayPtr way) const override;
	bool acceptNode(NodePtr node) const override;
	bool acceptAreaRelation(FeatureStore* store, RelationPtr relation) const override;
};


class IntersectsLinealFilter : public PreparedSpatialFilter
{
public:
	IntersectsLinealFilter(const Box& bounds, MCIndex&& index) :
		PreparedSpatialFilter(bounds, std::move(index)) {}

	bool accept(FeatureStore* store, FeaturePtr feature, FastFilterHint fast) const override;

protected:
	bool acceptWay(WayPtr way) const override;
	bool acceptNode(NodePtr node) const override;
	bool acceptAreaRelation(FeatureStore* store, RelationPtr relation) const override;
};


class IntersectsFilterFactory : public PreparedFilterFactory
{
public:
	const Filter* forPolygonal() override
	{ 
		return new IntersectsPolygonFilter(bounds(), buildIndex()); 
	}

	const Filter* forLineal() override
	{ 
		return new IntersectsLinealFilter(bounds(), buildIndex());
	}

	const Filter* forCoordinate(Coordinate point) override
	{ 
		return new ContainsPointFilter(point); 
	}
};
