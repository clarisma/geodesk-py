// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "WaySlicer.h"
#include "geom/Quadrant.h"
#include <cassert>

// TODO: ensure proper init order
WaySlicer::WaySlicer(WayPtr way) :
	iter_(way),
	first_(iter_.next()),
	second_(iter_.next()),
	hasMore_(true)
{
	assert(!first_.isNull());
	assert(!second_.isNull());
	assert(iter_.coordinatesRemaining() >= 0);
}

// TODO: could templatize this
void WaySlicer::slice(MonotoneChain* chain, int maxVertexes)
{
	int remaining = iter_.coordinatesRemaining(); 
	assert(remaining >= 0);
	Coordinate* pStart = chain->coords;
	Coordinate *p = pStart;
	*p++ = first_;
	*p++ = second_;
	Coordinate* pEnd = p + std::min(remaining, maxVertexes - 2);
	int32_t startY = first_.y;
	if (first_.y == second_.y)
	{
		// horizontal segment forms a separate MC
		hasMore_ = remaining > 0;
		first_ = second_;
		if(hasMore_) second_ = iter_.next();
		assert(iter_.coordinatesRemaining() >= 0);
	}
	else
	{
		int quadrant = Quadrant::quadrant(first_, second_);
		Coordinate prev = second_;
		if (remaining == 0)
		{
			hasMore_ = false;
		}
		else
		{
			for (;;)
			{
				Coordinate next = iter_.next();
				assert(!next.isNull());
				assert(iter_.coordinatesRemaining() >= 0);

				// We end th1e chain if Y-coordinate changes direction, or the next segment
				// if horizontal (We slice each horizontal segment individually)
				int newDirection = (next.y == prev.y ? 4 : 0) | Quadrant::quadrant(prev, next);
				if (newDirection != quadrant)
				{
					first_ = prev;
					second_ = next;
					hasMore_ = true;
					break;
				}
				*p++ = next;
				prev = next;
				if (p == pEnd)
				{
					hasMore_ = iter_.coordinatesRemaining() > 0;
					if (hasMore_)
					{
						first_ = next;
						second_ = iter_.next();
						assert(iter_.coordinatesRemaining() >= 0);
					}
					break;
				}
			}
		}
	}
	int32_t endY = (p-1)->y;
	chain->coordCount = static_cast<int32_t>(p - pStart);
}
