// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "WithinFilter.h"
#include "feature/polygon/PointInPolygon.h"
#include "feature/FastMemberIterator.h"
#include "geom/Mercator.h"
#include <common/util/log.h>

// TODO: anySegmentsCross
// Need to distinguish between "crossing" and "colinear"
// Right now, only conforms to intersects
// Need to check if candidate segment lies fully with reference segment
// If it exceeds the reference segment, it may lie outside
// Do we need to distinguish between shell and holes?
// for now, we dispense with edge check


bool WithinPolygonFilter::acceptWay(WayRef way) const
{
	// TODO: bbox check first to quickly determine if way does not 
	// interact with the bboxes of the monotone chains. If so,
	// just need to check inside/outside base on single point
	// (as well as check if way contains the filter polygon)

	return locateWayNodes(way) == 1; // && !anySegmentsCross(way);
}

bool WithinPolygonFilter::acceptNode(NodeRef node) const
{
	return index_.properlyContainsPoint(node.xy());
}


bool WithinPolygonFilter::acceptMembers(FeatureStore* store, RelationRef relation, RecursionGuard& guard) const
{
	return locateMembers(store, relation, guard) > 0;
		// all must lie inide or on boundary, and at least one point
		// must be inside
}


int WithinPolygonFilter::locateMembers(FeatureStore* store, RelationRef relation, RecursionGuard& guard) const
{
	int where = 0;
	FastMemberIterator iter(store, relation);
	for (;;)
	{
		FeatureRef member = iter.next();
		if (member.isNull()) break;
		int memberType = member.typeCode();
		if (memberType == 1)
		{
			WayRef memberWay(member);
			if (memberWay.isPlaceholder()) continue;
			int wayLocation = locateWayNodes(memberWay);
			if (wayLocation < 0) return -1;
			where = std::max(where, wayLocation);
		}
		else if (memberType == 0)
		{
			NodeRef memberNode(member);
			if (memberNode.isPlaceholder()) continue;
			int pointLocation = index_.locatePoint(memberNode.xy());
			if (pointLocation < 0) return -1;
			where = std::max(where, pointLocation);
		}
		else
		{
			assert(memberType == 2);
			RelationRef childRel(member);
			if (childRel.isPlaceholder() || !guard.checkAndAdd(childRel)) continue;
			int relLocation = locateMembers(store, childRel, guard);
			if (relLocation < 0) return -1;
			where = std::max(where, relLocation);
		}
	}
	return where;
}


bool WithinPolygonFilter::acceptAreaRelation(FeatureStore* store, RelationRef relation) const
{
	RecursionGuard guard(relation);
	return locateMembers(store, relation, guard) > 0;
}


bool WithinPolygonFilter::accept(FeatureStore* store, FeatureRef feature, FastFilterHint fast) const
{
	/*
	if (fast.turboFlags) return true;
	// TODO: Turbo-mode needs to check if feature lies fully within tile bounds
	*/
	return acceptFeature(store, feature);
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
int WithinPolygonFilter::locateWayNodes(WayRef way) const
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

