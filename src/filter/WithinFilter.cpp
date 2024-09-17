// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "WithinFilter.h"
#include "feature/FastMemberIterator.h"
#include "geom/Mercator.h"
#include "geom/polygon/PointInPolygon.h"
#include <common/util/log.h>
#include <geom/Centroid.h>

// TODO: anySegmentsCross
// Need to distinguish between "crossing" and "colinear"
// Right now, only conforms to intersects
// Need to check if candidate segment lies fully with reference segment
// If it exceeds the reference segment, it may lie outside
// Do we need to distinguish between shell and holes?
// for now, we dispense with edge check


bool WithinPolygonFilter::acceptWay(WayPtr way) const
{
	// TODO: bbox check first to quickly determine if way does not 
	// interact with the bboxes of the monotone chains. If so,
	// just need to check inside/outside base on single point
	// (as well as check if way contains the filter polygon)

	Box bounds = way.bounds();
	int loc = index_.maybeLocateBox(bounds);
	if (loc != 0) return loc > 0;

	// TODO: accept if location >= 0 for area within area
	// A within B is always true if A == B

	loc = locateWayNodes(way);
	if(loc < 0) return false;
	if(loc > 0) return true;

	// loc==0: all vertexes are on the boundary

	if(!way.isArea()) return false;

	// A linestring is not considered "within" if it lies entirely
	// on the boundary; it must have at least one node that lies on the
	// interior, hence we only accept 1 ("inside")

	// For ways that represent an area, we just ensure that none
	// of the way's geometry lies outside (> -1); we need to accept
	// 0 (on boundary) because two polygons which are geometrically equal 
	// are considered to be within each other
	// Exception (see https://github.com/clarisma/geodesk-py/issues/57):
	// If all vertexes of an area-way lie on the boundary of the test
	// area, we need to also check if the centroid of the candidate
	// area lies within the test area.

	Coordinate centroid = Centroid::ofWay(way);
	return index_.locatePoint(centroid) > 0;
}

bool WithinPolygonFilter::acceptNode(NodePtr node) const
{
	return index_.properlyContainsPoint(node.xy());
}


bool WithinPolygonFilter::acceptMembers(FeatureStore* store, RelationPtr relation, RecursionGuard& guard) const
{
	return locateMembers(store, relation, guard) > 0;
		// all must lie inide or on boundary, and at least one point
		// must be inside
}


int WithinPolygonFilter::locateMembers(FeatureStore* store, RelationPtr relation, RecursionGuard& guard) const
{
	int where = 0;
	FastMemberIterator iter(store, relation);
	for (;;)
	{
		FeaturePtr member = iter.next();
		if (member.isNull()) break;
		int memberType = member.typeCode();
		if (memberType == 1)
		{
			WayPtr memberWay(member);
			if (memberWay.isPlaceholder()) continue;
			int wayLocation = locateWayNodes(memberWay);
			if (wayLocation < 0) return -1;
			where = std::max(where, wayLocation);
		}
		else if (memberType == 0)
		{
			NodePtr memberNode(member);
			if (memberNode.isPlaceholder()) continue;
			int pointLocation = index_.locatePoint(memberNode.xy());
			if (pointLocation < 0) return -1;
			where = std::max(where, pointLocation);
		}
		else
		{
			assert(memberType == 2);
			RelationPtr childRel(member);
			if (childRel.isPlaceholder() || !guard.checkAndAdd(childRel)) continue;
			int relLocation = locateMembers(store, childRel, guard);
			if (relLocation < 0) return -1;
			where = std::max(where, relLocation);
		}
	}
	return where;
}


bool WithinPolygonFilter::acceptAreaRelation(FeatureStore* store, RelationPtr relation) const
{
	// We only check ways (i.e. ignore label nodes and sub-areas)

	FastMemberIterator iter(store, relation);
	for (;;)
	{
		FeaturePtr member = iter.next();
		if (member.isNull()) break;
		if (member.isWay())
		{
			WayPtr memberWay(member);
			if (memberWay.isPlaceholder()) continue;
			if (locateWayNodes(memberWay) < 0) return false;
		}
	}

	// TODO: Currently, areas without ways will be accepted (we should
	// return false instead if there were no non-placeholder ways);
	// however, this needs to be fixed in the GOL Tool and therefore
	// won't be an issue
	// See geodesk-py#30, gol-tool#107

	return true;
}


bool WithinPolygonFilter::accept(FeatureStore* store, FeaturePtr feature, FastFilterHint fast) const
{
	if (fast.turboFlags)
	{
		if ((feature.flags() &
			(FeatureFlags::MULTITILE_NORTH | FeatureFlags::MULTITILE_WEST)) == 0)
		{
			// If feature lies completely within the current tile,
			// we can fast-accept it

			if (feature.minY() >= fast.tile.bottomY() &&
				feature.maxX() <= fast.tile.rightX())
			{
				return true;
			}
		}
	}
	return acceptFeature(store, feature);
}


int WithinPolygonFilter::acceptTile(Tile tile) const
{
	Box tileBounds = tile.bounds();
	int loc = index_.locateBox(tileBounds);
	if (loc > 0) return 1; 
		// TODO: Don't use 1 to indicate tile acceleration, use enum constant
	return loc; 
}

/*
bool WithinPolygonFilter::accept(FeatureStore* store, FeatureRef feature, FastFilterHint fast) const
{
	// if (fast.turboFlags) return true;
	// TODO: Turbo-mode needs to check if feature lies fully within tile bounds

	int type = feature.typeCode();
	if (type == 1)
	{
		// TODO: bbox check first to quickly determine if way does not 
		// interact with the bboxes of the monotone chains. If so,
		// just need to check inside/outside base on single point
		// (as well as check if way contains the filter polygon)

		WayRef way(feature);
		return locateWayNodes(way) == 1; // && !anySegmentsCross(way);
	}
	else if (type == 0)
	{
		// Node must lie in interior; if it lies on boundary, it's not "within"
		NodeRef node(feature);
		return index_.properlyContainsPoint(node.xy());
	}

	// Relation
	RelationRef relation(feature);
	int where = 0;

	// TODO: cannot use regular iterator because it is not threadsafe
	// (it creates the strings for roles)
	FastMemberIterator iter(store, relation);
	for (;;)
	{
		FeatureRef member = iter.next();
		if (member.isNull()) break;
		int memberType = member.typeCode();
		if (memberType == 1)
		{
			// TODO: bbox check first!
			
			WayRef way(member);
			int wayLocation = locateWayNodes(way);
			if (wayLocation < 0) return false;
			where = std::max(where, wayLocation);
			// if (anySegmentsCross(way)) return false;
		}
		else if (memberType == 0)
		{
			// Node
			NodeRef memberNode(member);
			int pointLocation = index_.locatePoint(memberNode.xy());
			if (pointLocation < 0) return false;
			where = std::max(where, pointLocation);
		}
		// TODO: relation -- but careful, can be circular
		// TODO: need to check if candidate encloses polygon ??
	}
	return where > 0;
}

*/

// -1 = outside
//  0 = completely on boundary
//  1 = inside
int WithinPolygonFilter::locateWayNodes(WayPtr way) const
{
	int where = 0;
	WayCoordinateIterator iter;
	iter.start(way, 0);
	for (;;)
	{
		Coordinate c = iter.next();
		if (c.isNull()) break;

		int pointLocation = index_.locatePoint(c);
		if (pointLocation < 0) return pointLocation;
		where = std::max(where, pointLocation);
	}
	return where;
}

