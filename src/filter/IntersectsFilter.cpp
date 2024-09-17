// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "IntersectsFilter.h"
#include "geom/polygon/PointInPolygon.h"

static bool chainContainedByAreaWay(const RTree<const MonotoneChain>::Node* node, WayPtr way)
{
	if (!way.bounds().containsSimple(node->bounds)) return false;
	PointInPolygon tester(node->item()->first());
	tester.testAgainstWay(way);
	return tester.isInside();
}

struct StoredRelation
{
	FeatureStore* const store;
	RelationPtr relation;
};

// TODO: use a list of representative points? (one for each polygon in multipolygon)

static bool chainContainedByAreaRelation(
	const RTree<const MonotoneChain>::Node* node, const StoredRelation* storedRel)
{
	RelationPtr relation = storedRel->relation;
	if (!relation.bounds().containsSimple(node->bounds)) return false;
	PointInPolygon tester(node->item()->first());
	tester.testAgainstRelation(storedRel->store, relation);
	return tester.isInside();
}


bool IntersectsPolygonFilter::acceptWay(WayPtr way) const
{
	Box bounds = way.bounds();
	int loc = index_.maybeLocateBox(bounds);
	if (loc != 0) return loc > 0;

	if (wayIntersectsPolygon(way)) return true;
	if(way.isArea())
	{
		// - If way appears to not intersect, perform this final test:
		//   - Find all MCs whose bbox intersects candidate bbox:
		//     - IF MC bbox contained in candidate bbox:
		//       - Check if first coordinate lies inside candidate polygon
		//         - If so, the features intersect
		// - OR: Perform this before the chain-crossing tests?

		return index_.findChains(bounds, chainContainedByAreaWay, way);
	}
	return false;
}

bool IntersectsPolygonFilter::acceptNode(NodePtr node) const
{
	return index_.containsPoint(node.xy());
}

bool IntersectsPolygonFilter::acceptAreaRelation(FeatureStore* store, RelationPtr relation) const
{
	// TODO: check ways only
	RecursionGuard guard(relation);
	if (acceptMembers(store, relation, guard)) return true;
	
	// If candidate does not lie inside test (multi)polygon, and none
	// of their boundaries cross, check if the candidate contains any
	// part of the test geometry

	const StoredRelation storedRel{ store, relation };
	return index_.findChains(relation.bounds(), chainContainedByAreaRelation, 
		&storedRel);
}


bool IntersectsPolygonFilter::accept(FeatureStore* store, FeaturePtr feature, FastFilterHint fast) const
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


bool IntersectsLinealFilter::acceptWay(WayPtr way) const
{
	if(anySegmentsCross(way)) return true;
	if (!way.isArea()) return false;
	return index_.findChains(way.bounds(), chainContainedByAreaWay, way);
}

bool IntersectsLinealFilter::acceptNode(NodePtr node) const
{
	return index_.pointOnBoundary(node.xy());
}

bool IntersectsLinealFilter::acceptAreaRelation(FeatureStore* store, RelationPtr relation) const
{
	// TODO: check ways only
	RecursionGuard guard(relation);
	if (acceptMembers(store, relation, guard)) return true;
	
	const StoredRelation storedRel{ store, relation };
	return index_.findChains(relation.bounds(), 
		chainContainedByAreaRelation, &storedRel);
}


bool IntersectsLinealFilter::accept(FeatureStore* store, FeaturePtr feature, FastFilterHint fast) const
{
	if (fast.turboFlags) return true;
	return acceptFeature(store, feature);
}

