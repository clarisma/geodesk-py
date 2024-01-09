// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PreparedSpatialFilter.h"
#include "geom/mc/WaySlicer.h"

bool PreparedSpatialFilter::anyNodesInPolygon(WayRef way) const
{
	WayCoordinateIterator iter;
	iter.start(way, 0);
	for (;;)
	{
		Coordinate c = iter.next();
		if (c.isNull()) return false;
		return index_.containsPoint(c);
	}
}

bool PreparedSpatialFilter::anySegmentsCross(WayRef way) const
{
	WaySlicer slicer(way);
	uint8_t buf[MonotoneChain::storageSize(MAX_CANDIDATE_MC_LENGTH)];
	do
	{
		MonotoneChain* chain = reinterpret_cast<MonotoneChain*>(&buf);
		slicer.slice(chain, MAX_CANDIDATE_MC_LENGTH);
		chain->normalize();
		if (index_.intersects(chain)) return true;
	}
	while (slicer.hasMore());
	return false;
}

bool PreparedSpatialFilter::wayIntersectsPolygon(WayRef way) const
{

	// First, check if any nodes lie inside the polygon or on its boundary
	// if (anyNodesInPolygon(way)) return true;
	 
	// Changed: We only need to check the first node
	// But performance impact appears to be negligible
	
	WayCoordinateIterator iter;
	iter.start(way, 0);
	if(index_.containsPoint(iter.next())) return true;
		 
	// Then check if any of the way's line segments cross any line segments of 
	// the polygon
	return anySegmentsCross(way);
	// TODO: need to check if area-way encloses polygon
	// must be area
	// bbox must be >= polygon bbox

	// TODO: It may be cheaper to check only if the first point lies inside
	// the test geometry
	// If first point lies outside, the way can only intersect if any
	// segments cross
}
