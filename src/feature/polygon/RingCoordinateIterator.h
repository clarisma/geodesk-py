// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "Ring.h"

// TODO: respect backward/forward

class RingCoordinateIterator
{
public:
	RingCoordinateIterator(const Polygonizer::Ring* ring) :
		remaining_(ring->vertexCount())
	{
		Polygonizer::Segment* seg = ring->firstSegment_;
		nextSegment_ = seg->next;
		if (seg->backward)
		{
			p_ = &seg->coords[seg->vertexCount-1];
			pEndCoords_ = &seg->coords[-1];
			direction_ = -1;
		}
		else
		{
			p_ = seg->coords;
			pEndCoords_ = p_ + seg->vertexCount;
			direction_ = 1;
		}
	}

	int coordinatesRemaining() const { return remaining_; }
	Coordinate next()
	{
		assert(remaining_ > 0);
		Coordinate c = *p_;
		p_ += direction_;
		if (p_ == pEndCoords_)
		{
			if (nextSegment_)
			{
				if (nextSegment_->backward)
				{
					p_ = &nextSegment_->coords[nextSegment_->vertexCount - 2];
					pEndCoords_ = &nextSegment_->coords[-1];
					direction_ = -1;
				}
				else
				{
					p_ = &nextSegment_->coords[1];
					pEndCoords_ = &nextSegment_->coords[nextSegment_->vertexCount];
					direction_ = 1;
				}
				nextSegment_ = nextSegment_->next;
			}
			// TODO: safeguard against caller misuse
		}
		remaining_--;
		return c;
	}

private:
	int remaining_;
	int direction_;
	Polygonizer::Segment* nextSegment_;
	Coordinate* p_;
	Coordinate* pEndCoords_;
};


