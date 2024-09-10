// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "geom/mc/MCIndexBuilder.h"
#include "ContainsPointFilter.h"
#include "PreparedSpatialFilter.h"
#include "PreparedFilterFactory.h"

class CrossesFilter : public PreparedSpatialFilter
{
public:
	CrossesFilter(FeatureTypes accepted, const Box& bounds, MCIndex&& index) :
		PreparedSpatialFilter(bounds, std::move(index)) 
	{
		flags_ |= FilterFlags::FAST_TILE_FILTER;
		acceptedTypes_ = accepted;
	}

	bool accept(FeatureStore* store, FeaturePtr feature, FastFilterHint fast) const override;
	int acceptTile(Tile tile) const override;

protected:
	bool acceptWay(WayPtr way) const override;
	bool acceptNode(NodePtr node) const override;
	bool acceptAreaRelation(FeatureStore* store, RelationPtr relation) const override;
};



class CrossesFilterFactory : public PreparedFilterFactory
{
public:
	const Filter* forPolygonal() override
	{ 
		return new CrossesFilter(FeatureTypes::ALL & 
			~FeatureTypes::AREAS & ~FeatureTypes::NODES,
			bounds(), buildIndex()); 
	}

	const Filter* forLineal() override
	{ 
		return new CrossesFilter(FeatureTypes::ALL & ~FeatureTypes::NODES,
			bounds(), buildIndex());
	}

	const Filter* forNonAreaRelation(FeatureStore* store, RelationPtr rel) override
	{
		return forLineal();
	}

};
