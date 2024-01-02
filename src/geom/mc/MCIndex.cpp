// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "MCIndex.h"
#include "MonotoneChain.h"
#include <common/util/log.h>

bool MCIndex::intersects(const MonotoneChain* mc) const
{
	assert(mc->isNormalized());
	// TODO: could use a faster version of bounds for normalized MC
	return index_.search(mc->bounds(), intersectsChain, mc);
}


bool MCIndex::intersectsChain(const RTree<const MonotoneChain>::Node* node, const MonotoneChain* candidate)
{
	return candidate->intersects(node->item());
}


bool MCIndex::properlyContainsPoint(Coordinate c) const
{
	PointLocationClosure closure(c);
	// We use a bounding box that represents a line that starts at the
	// candidate point and extends to the easternmost edge of the map;
	// we use this line to find all the 
	index_.search(
		Box(c.x, c.y, std::numeric_limits<int32_t>::max(), c.y),
		countCrossings, &closure);
	return closure.location.isInside();
	// TODO: verify that no further crossing checks are performed
	// after we've determined that a point lies on a boundary
	// If point lies on a boundary, crossing count is set to 0 (even),
	// (so we don't have to perform two checks here)
}

bool MCIndex::containsPoint(Coordinate c) const
{
	PointLocationClosure closure(c);
	index_.search(
		Box(c.x, c.y, std::numeric_limits<int32_t>::max(), c.y),
		countCrossings, &closure);
	return closure.location.isInside() || closure.location.isOnBoundary();
}

bool MCIndex::pointOnBoundary(Coordinate c) const
{
	PointLocationClosure closure(c);
	index_.search(
		Box(c.x, c.y, c.x, c.y),
		countCrossings, &closure);
	return closure.location.isOnBoundary();
}


bool MCIndex::countCrossings(const RTree<const MonotoneChain>::Node* node,
	PointLocationClosure* closure)
{
	Coordinate c = closure->point;
	if (c.y == node->bounds.maxY())
	{
		if (c.y == node->bounds.minY() && c.x >= node->bounds.minX())
		{
			// The point lies on a horizontal segment
			closure->location.setOnBoundary();
			return true;	// stop r-tree search
		}

		// Check if point is coincident with the end vertex of the chain
		if (c.x == node->item()->last().x)
		{
			closure->location.setOnBoundary();
			return true;    // stop r-tree search
		}

		// Otherwise, we disregard a ray that crosses through the end
		// vertex of the monotone chain, because otherwise we'd risk
		// double-counting a crossing through the start vertex of 
		// a connected chain 
		// - If two chains share the same end vertex, we count neither, 
		//   which is fine since we're only interested in odd or even 
		//   count (odd means "inside")
		// - if point lies on a connected horizontal MC, we address 
		//   this in the check above
	}
	else
	{
		if (c.x < node->bounds.minX())
		{
			// If point lies to the left of the MC's bounding box,
			// it is guaranteed to cross the MC
			closure->location.addCrossing();
		}
		else
		{
			const Coordinate* p = node->item()->findSegmentForY(c.y);
			Coordinate start = *p;
			Coordinate end = *(p + 1);
			assert (c.y >= start.y && c.y <= end.y);
			double crossProduct = 
				((double)c.y - (double)start.y) * ((double)end.x - (double)start.x) -
				((double)c.x - (double)start.x) * ((double)end.y - (double)start.y);

			if (crossProduct == 0)
			{
				closure->location.setOnBoundary();
				return true;
			}
			else if (crossProduct > 0)
			{
				closure->location.addCrossing();
			}
		}
	}
	return false; // keep going
}

bool MCIndex::intersects(const Box* box) const
{
	return false; // TODO
}

