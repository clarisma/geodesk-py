// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "IntersectsFilter.h"
#include "feature/polygon/PointInPolygon.h"
#include "feature/FastMemberIterator.h"

bool IntersectsPolygonFilter::acceptWay(WayRef way) const
{
	Box bounds = way.bounds();
	int loc = index_.maybeLocateBox(bounds);
	if (loc != 0) return loc > 0;

	if (wayIntersectsPolygon(way)) return true;
	if (way.isArea() && way.bounds().containsSimple(bounds_))
	{
		// check if representative point lies inside
		// TODO: No, this test is not sufficient; we could also have a
		// multi-polygon test geometry, and the candidate geometry only contains
		// one of the polygons (which may not have the representative point)
		// Must check entire test geom!

		// Also, containsSimple does not work for the case where test geometry
		// is a multi-polygon

		// Proposed:
		// - If way appears to not intersect, perform this final test:
		//   - Find all MCs whose bbox intersects candidate bbox:
		//     - IF MC bbox contained in candidate bbox:
		//       - Check if first coordinate lies inside candidate polygon
		//         - If so, the features intersect
		// - OR: Perform this before the chain-crossing tests?

		PointInPolygon tester(index_.representativePoint());
		tester.testAgainstWay(way);
		return tester.isInside();
	}
	return false;
}

bool IntersectsPolygonFilter::acceptNode(NodeRef node) const
{
	return index_.containsPoint(node.xy());
}

bool IntersectsPolygonFilter::acceptAreaRelation(FeatureStore* store, RelationRef relation) const
{
	RecursionGuard guard(relation);
	if (acceptMembers(store, relation, guard)) return true;
	if (!relation.bounds().containsSimple(bounds_)) return false;
	// check if representative point lies inside area relation
	// TODO: No, this test is not sufficient; we could also have a
	// multi-polygon test geometry, and the candidate geometry only contains
	// one of the polygons (which may not have the representative point)
	// Must check entire test geom!
	PointInPolygon tester(index_.representativePoint());
	tester.testAgainstRelation(store, relation);
	return tester.isInside();
}


bool IntersectsPolygonFilter::accept(FeatureStore* store, FeatureRef feature, FastFilterHint fast) const
{
	if (fast.turboFlags) return true;
	return acceptFeature(store, feature);
}

int IntersectsPolygonFilter::acceptTile(Tile tile) const
{
	Box tileBounds = tile.bounds();
	int loc = index_.locateBox(tileBounds);
	if (loc > 0) return 1;
	// TODO: Don't use 1 to indicate tile acceleration, use enum constant
	return loc;
}


bool IntersectsLinealFilter::acceptWay(WayRef way) const
{
	// TODO: We also need to check if an area-way contains the test geometry!
	return anySegmentsCross(way);
}

bool IntersectsLinealFilter::acceptNode(NodeRef node) const
{
	return index_.pointOnBoundary(node.xy());
}

bool IntersectsLinealFilter::acceptAreaRelation(FeatureStore* store, RelationRef relation) const
{
	// TODO: We also need to check if an area-relation contains the test geometry!
	RecursionGuard guard(relation);
	return acceptMembers(store, relation, guard);
}


bool IntersectsLinealFilter::accept(FeatureStore* store, FeatureRef feature, FastFilterHint fast) const
{
	if (fast.turboFlags) return true;
	return acceptFeature(store, feature);
}

