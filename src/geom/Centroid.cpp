// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "Centroid.h"
#include <cstring>
#include "feature/FeatureStore.h"
#include "feature/polygon/Polygonizer.h"
#include "feature/polygon/RingCoordinateIterator.h"



Coordinate Centroid::ofWay(WayPtr way)
{
	WayCoordinateIterator iter(way);
	if (way.isArea())
	{
		Centroid::Areal centroid;
		centroid.addRing(iter, true);
		return centroid.centroid();
	}
	else
	{
		Centroid::Lineal centroid;
		centroid.addLineSegments(way);
		return centroid.centroid();
	}
}


void Centroid::Areal::addAreaRelation(FeatureStore* store, RelationPtr relation)
{
	Polygonizer polygonizer;
	polygonizer.createRings(store, relation);
	const Polygonizer::Ring* ring = polygonizer.outerRings();
	while (ring)
	{
		RingCoordinateIterator iter(ring);
		addRing(iter, true);
		ring = ring->next();
	}
	ring = polygonizer.innerRings();
	while (ring)
	{
		RingCoordinateIterator iter(ring);
		addRing(iter, false);
		ring = ring->next();
	}
}


Coordinate Centroid::ofRelation(FeatureStore* store, RelationPtr relation)
{
	if (relation.isArea())
	{
		Centroid::Areal centroid;
		centroid.addAreaRelation(store, relation);
		if (centroid.isEmpty()) return relation.bounds().center();
			// If the relation is degenerate to the point where no rings could
			// be built, return the cneter of its bbox. We can't use centroid()
			// because the sum of areas will be zero and results in division-by-zero
		return centroid.centroid();
	}
	else
	{
		Centroid centroid;
		RecursionGuard guard(relation);
		centroid.addRelation(store, relation, guard);
		if (!centroid.areal_.isEmpty()) return centroid.areal_.centroid();
		if (!centroid.lineal_.isEmpty()) return centroid.lineal_.centroid();
		if (!centroid.puntal_.isEmpty()) return centroid.puntal_.centroid();
		return relation.bounds().center();
	}
}


void Centroid::addWay(WayPtr way)
{
	WayCoordinateIterator iter(way);
	if (way.isArea())
	{
		areal_.addRing(iter, true);
	}
	else
	{
		lineal_.addLineSegments(way);
	}
}


void Centroid::addRelation(FeatureStore* store, RelationPtr rel, RecursionGuard& guard)
{
	FastMemberIterator iter(store, rel);
	for (;;)
	{
		FeaturePtr member = iter.next();
		if (member.isNull()) break;
		int memberType = member.typeCode();
		if (memberType == 1)
		{
			WayPtr memberWay(member);
			if(!memberWay.isPlaceholder()) addWay(memberWay);
		}
		else if (memberType == 0)
		{
			// Node
			NodePtr memberNode(member);
			if(!memberNode.isPlaceholder()) puntal_.addPoint(memberNode.xy());
		}
		else
		{
			// Relation
			RelationPtr childRel(member);
			if (!childRel.isPlaceholder() && guard.checkAndAdd(childRel))
			{
				addRelation(store, childRel, guard);
			}
		}
	}
}


void Centroid::Lineal::addLineSegments(WayPtr way)
{
	WayCoordinateIterator iter(way);
	Coordinate c = iter.next();
	double x1 = c.x;
	double y1 = c.y;
	for (int count = iter.coordinatesRemaining(); count > 0; count--)
	{
		c = iter.next();
		double x2 = c.x;
		double y2 = c.y;
		double xDelta = x1 - x2;
		double yDelta = y1 - y2;
		double d = sqrt(xDelta * xDelta + yDelta * yDelta);
		totalLength_ += d;
		lineCentroidX_ += (x1 + x2) * d;
		lineCentroidY_ += (y1 + y2) * d;
		x1 = x2;
		y1 = y2;
	}
}


void Centroid::Puntal::addPoint(Coordinate point)
{
	pointCentroidX_ += point.x;
	pointCentroidY_ += point.y;
	pointCount_++;
}


Coordinate Centroid::ofFeature(FeatureStore* store, FeaturePtr feature)
{
	if (feature.isWay())
	{
		return ofWay(WayPtr(feature));
	}
	if (feature.isNode()) return NodePtr(feature).xy();
	assert(feature.isRelation());
	return ofRelation(store, RelationPtr(feature));
}
