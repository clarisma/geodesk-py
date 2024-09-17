// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "feature/FastMemberIterator.h"
#include "feature/WayCoordinateIterator.h"
#include "geom/LineSegment.h"


class PointInPolygon
{
public:
	// TODO: class is not really used here, defined for Polygonizer
	// TODO: These constants differ from other cases, which use -1/0/1
	enum class Location
	{
		OUTSIDE = 0,
		INSIDE = 1,
		BOUNDARY = 2
	};

	PointInPolygon(Coordinate pt) : point_(pt), crossingCount_(0) {}
	bool isInside() const { return crossingCount_ & 2; }
		// we use & 2 since we use +2 for proper crossings, 
		// +1 for crossings through a vertex
		// (If count is uneven, we have a non-polygonal geometry)

	/**
	 * Adds the number of times a ray cast from the test point crosses
	 * the given way. Shortcuts if point lies on boundary.
	 * 
	 * @return true if point lies on boundary, else false.
	 */
	bool testAgainstWay(const WayPtr way)
	{
		Box bounds = way.bounds();
		if (point_.y < bounds.minY() || point_.y > bounds.maxY())
		{
			// Y of point is outside of Y range of way's bbox, hence
			// no intersection is possible.
			return false;
		}
		WayCoordinateIterator iter(way);
		Coordinate prev = iter.next();
		for (;;)
		{
			Coordinate next = iter.next();
			if (next.isNull()) break;

			// we normalize the vector so it always points upwards
			Coordinate start = prev.y < next.y ? prev : next;
			Coordinate end = prev.y < next.y ? next : prev;
			
			if (point_.y >= start.y && point_.y <= end.y)
			{
				// TODO: this could be more efficient

				int orientation = LineSegment::orientation(start, end, point_);
				if (orientation == 0) return true;
				crossingCount_ += orientation > 0 ?
					((point_.y == start.y || point_.y == end.y) ? 1 : 2) : 0;
				// We count a ray crossing through a vertex as one-half
			}
			prev = next;
		}
		return false;
	}

	bool testAgainstRelation(FeatureStore* store, const RelationPtr relation)
	{
		FastMemberIterator iter(store, relation);
		for (;;)
		{
			FeaturePtr member = iter.next();
			if (member.isNull()) break;
			if (!member.isWay()) continue;
			WayPtr way(member);
			if (way.isPlaceholder()) continue;
			// TODO: do we need to check for inner/outer role?
			// Can we establish that area relations must 
			// not have non-boundary ways?
			if (testAgainstWay(way)) return true;
		}
		return false;
	}

private:
	Coordinate point_;
	uint32_t crossingCount_;
};


