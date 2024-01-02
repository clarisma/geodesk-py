// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "MonotoneChain.h"
#include <algorithm>
#include "geom/LineSegment.h"

static_assert(sizeof(MonotoneChain) == 12,
	"Compiler doesn't properly pack structs");

/**
* Finds the first segment in the given monotone chain for which y <= segment.endY.
* The function assumes that y is greater or equal to the lowest Y of the chain,
* and <= the highest Y of the chain (i.e. it falls into the Y-interval of the chain).
*
* @return pointer to the segment's start vertex
*/

const Coordinate* MonotoneChain::findSegmentForY(int32_t y) const
{
	int start = 1;				// skip first coordinate in search
	int end = coordCount - 1;
	while (start <= end)
	{
		int mid = start + (end - start) / 2;  // +1 to ensure we move forward in case of ties
			// TODO: modified this
		if (coords[mid].y < y)
		{
			start = mid + 1;  // Move the starting boundary forward
		}
		else
		{
			end = mid - 1;  // Move the ending boundary backward
		}
		// At the end of the binary search, start and end will converge on the desired index
	}

	assert(y >= coords[start-1].y && y <= coords[start].y);

	return &coords[start - 1];   // TODO: check
}

/**
 * @brief Checks whether two monotone chains intersect.
 *
 * Given two monotone chains that increment monotonically along the Y axis,
 * this function checks if they intersect using a linearithmic approach.
 *
 * The monotone chains are represented as arrays of `int32_t` values:
 * - The first `int32_t` value represents the number of vertices in the chain.
 * - The subsequent `int32_t` values are interleaved x/y coordinates of the vertices.
 *
 * Algorithm Overview:
 * 1. Determine which chain starts at a lower Y-coordinate (`chainLow`) and
 *    which starts higher (`chainHigh`).
 * 2. Use binary search on `chainLow` to find the first segment whose end Y-coordinate
 *    is equal to or above the starting Y-coordinate of `chainHigh`.
 * 3. From this found position, iterate over the segments of both chains, checking
 *    for intersections.
 *    a. For each step, test the intersection of the current segment of `chainLow`
 *       with the current segment of `chainHigh`.
 *    b. If an intersection is found, return true.
 *    c. After each segment intersection check, advance the pointer of the chain
 *       whose current segment's end Y-coordinate is lower. If any pointer reaches
 *       the end of its chain without finding an intersection, the chains do not
 *       intersect.
 *
 * @param chain1   pointer to the first monotone chain.
 * @param chain2   pointer to the second monotone chain.
 * @return Returns `true` if the chains intersect, `false` otherwise.
 */

 // TODO: the binary search may not work correctly if the chain includes a
 // segment that runs parallel to the x-axis (its start-Y and end-Y are the same)
 // May need to treat such a segment as its own chain (better for spatial indexing
 // anyway, as such a segment needlessly inflates the bbox)

// If we use quadrant-MCs, we only need to check a single segment if their 
//  quadrants are different (i.e. one moves east and one moves west), as those
//  chains can only intersect in one segment 
//  (y direction always moves north)

bool MonotoneChain::intersects(const MonotoneChain* other) const
{
	const MonotoneChain* chain1 = this;
	const MonotoneChain* chain2 = other;

	int32_t startY1 = chain1->coords[0].y;
	int32_t startY2 = chain2->coords[0].y;
	if (chain2->coords[0].y > startY1)
	{
		std::swap(chain1, chain2);
		std::swap(startY1, startY2);
	}

	const Coordinate* p1 = chain1->coords;
	const Coordinate* pEnd1 = chain1->coords + chain1->coordCount;
	const Coordinate* p2 = chain2->findSegmentForY(startY1);
	const Coordinate* pEnd2 = chain2->coords + chain2->coordCount;
	Coordinate start1 = *p1++;
	Coordinate end1 = *p1++;
	Coordinate start2 = *p2++;
	Coordinate end2 = *p2++;

	for (;;)
	{
		// check for line segment intersection
		if (LineSegment::linesIntersect(start1, end1, start2, end2))
		{
			return true;
		}

		if (end1.y < end2.y)
		{
			if (p1 == pEnd1) return false;
			start1 = end1;
			end1 = *p1++;
		}
		else
		{
			if (p2 == pEnd2) return false;
			start2 = end2;
			end2 = *p2++;
		}
		// TODO: Is endY1 == endY2 a special case?
		// Right now, we arbitrarily move one pointer or the other, could this
		// impact robustness?
	}
}


void MonotoneChain::copyCoordinates(Coordinate* dest, int direction) const
{
	const Coordinate* p = coords;
	const Coordinate* end = coords + coordCount;
	do
	{
		*dest = *p++;
		dest += direction;
	} 
	while (p < end);
}


void MonotoneChain::copyNormalized(MonotoneChain* dest) const
{
	int32_t startY = coords[0].y;
	int32_t endY = coords[1].y;
	copyCoordinates(
		endY < startY ? (dest->coords + coordCount - 1) : dest->coords,
		endY < startY ? -1 : 1);
	dest->coordCount = coordCount;
}

void MonotoneChain::reverse()
{
	Coordinate* front = coords;
	Coordinate* back = coords + coordCount - 1;
	do
	{
		std::swap(*front++, *back--);
	} 
	while (front < back);
}


/*
Coordinate MonotoneChain::createFromWay(Coordinate start, int wayFlags, WayCoordinateIterator& iter, int maxVertexes)
{
	Coordinate* p = coords;
	const Coordinate* pEnd = p + std::min(maxVertexes, iter.coordinatesRemaining(wayFlags) + 1);
	*p++ = start;
	while (p < pEnd)
	{
		Coordinate end = iter.next();

	}
}
*/