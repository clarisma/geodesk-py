// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "CrossesFilter.h"
#include "feature/FastMemberIterator.h"


int CrossesFilter::acceptTile(Tile tile) const
{
	return index_.locateBox(tile.bounds()) == 0 ? 0 : -1;
}

bool CrossesFilter::acceptWay(WayPtr way) const
{
	// If the way's bounding box does not intersect any MC bboxes,
	// then it is impossible for its geometry to cross 
	if (!index_.intersectsBoxBoundary(way.bounds())) return false;
	return anySegmentsCross(way);
}

bool CrossesFilter::acceptNode(NodePtr node) const
{
	return false;
}

bool CrossesFilter::acceptAreaRelation(FeatureStore* store, RelationPtr relation) const
{
	RecursionGuard guard(relation);
	return acceptMembers(store, relation, guard);
}


bool CrossesFilter::accept(FeatureStore* store, FeaturePtr feature, FastFilterHint fast) const
{
	// "crosses" has no turbo-mode; only tiles that interact with segments
	// are examined, all others are rejected
	return acceptFeature(store, feature);
}

