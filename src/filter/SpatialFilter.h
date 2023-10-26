// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "Filter.h"
#include "feature/Node.h"
#include "feature/Relation.h"
#include "feature/Way.h"

class SpatialFilter : public Filter
{
public:
    SpatialFilter() :
        Filter(FilterFlags::USES_BBOX, FeatureTypes::ALL),
        bounds_(Box::ofWorld()) {}
    SpatialFilter(const Box& bounds) :
        Filter(FilterFlags::USES_BBOX, FeatureTypes::ALL),
        bounds_(bounds) {}
    SpatialFilter(int flags, FeatureTypes acceptedTypes, const Box& bounds) :
        Filter(flags, acceptedTypes),
        bounds_(bounds) {}

    const Box& bounds() const { return bounds_; }

protected:
    bool acceptFeature(FeatureStore* store, FeatureRef feature) const;
    
    virtual bool acceptWay(WayRef way) const { return false; }
    virtual bool acceptNode(NodeRef node) const { return false; }
    virtual bool acceptAreaRelation(FeatureStore* store, RelationRef relation) const { return false; }
    virtual bool acceptMembers(FeatureStore* store, RelationRef relation, RecursionGuard& guard) const;
    Box bounds_;
};