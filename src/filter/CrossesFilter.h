// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "feature/Relation.h"
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
		acceptedTypes_ = accepted;
	}

	bool accept(FeatureStore* store, FeatureRef feature, FastFilterHint fast) const override;

protected:
	bool acceptWay(WayRef way) const override;
	bool acceptNode(NodeRef node) const override;
	bool acceptAreaRelation(FeatureStore* store, RelationRef relation) const override;
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

	const Filter* forNonAreaRelation(FeatureStore* store, RelationRef rel) override
	{
		return forLineal();
	}

};
