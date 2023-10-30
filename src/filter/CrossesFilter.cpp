// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "CrossesFilter.h"
#include "feature/FastMemberIterator.h"



bool CrossesFilter::acceptWay(WayRef way) const
{
	return anySegmentsCross(way);
}

bool CrossesFilter::acceptNode(NodeRef node) const
{
	return false;
}

bool CrossesFilter::acceptAreaRelation(FeatureStore* store, RelationRef relation) const
{
	RecursionGuard guard(relation);
	return acceptMembers(store, relation, guard);
}


bool CrossesFilter::accept(FeatureStore* store, FeatureRef feature, FastFilterHint fast) const
{
	// if (fast.turboFlags) return true;	
		// TODO: fast-reject single-tile features
	return acceptFeature(store, feature);
}

