// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "CrossesFilter.h"
#include "feature/FastMemberIterator.h"


int CrossesFilter::acceptTile(Tile tile) const
{
	return 0; // TODO
	/*
	Box tileBounds = tile.bounds();
	if (index_.intersectsBoxBoundary(tileBounds)) return 0;
	if(tileBounds.containsSimple(bounds_))
	{
		// TODO: Need to test tile if it contains the test geometry
	}
	return -1;
	*/
}

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
	// "crosses" has no turbo-mode; only tiles that interact with segments
	// are examined, all others are rejected
	return acceptFeature(store, feature);
}

