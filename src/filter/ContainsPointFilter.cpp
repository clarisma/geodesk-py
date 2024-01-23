// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "ContainsPointFilter.h"
#include "feature/polygon/PointInPolygon.h"

bool ContainsPointFilter::accept(FeatureStore* store, FeatureRef feature, FastFilterHint /* ignored */) const
{
	if (feature.isArea())
	{
		if (feature.isWay())
		{
			PointInPolygon tester(point_);
			// point is on boundary or inside (odd number of crossings)
			return tester.testAgainstWay(WayRef(feature)) || tester.isInside();
		}
		else
		{
			PointInPolygon tester(point_);
			// point is on boundary or inside (odd number of crossings)
			return tester.testAgainstRelation(store, RelationRef(feature)) || tester.isInside();
		}
	}
	else if (feature.isWay())
	{
		WayRef way(feature);
		WayCoordinateIterator iter(way);
		Coordinate start = iter.next();
		for (;;)
		{
			Coordinate end = iter.next();
			if (end.isNull()) break;
			if (LineSegment::orientation(start, end, point_) == 0)
			{
				// test point lies on line segment, which means
				// the linear way "contains" the point
				return true;
			}
			start = end;
		}
		return false;
	}
	else if (feature.isNode())
	{
		// A node contains another node only if their coordinates are the same
		NodeRef node(feature);
		return node.xy() == point_;
	}
	// TODO: non-area relations
	return false;
}
