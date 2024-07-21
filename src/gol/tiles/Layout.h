// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <common/data/FixedQueue.h>
#include "TElement.h"

class TileKit;

class Layout
{
public:
	Layout(TileKit& tile);

	TileKit& tile() const { return tile_; }
	int32_t size() { return pos_; }
	void place(TElement* elem);
	void flush();

	void put(TElement* elem)
	{
		put(elem, elem->alignedLocation(pos_));
	}

	int32_t size() const { return pos_; }
	TElement* first() const { return placed_.first(); }

	void addBodyElement(TElement* elem)
	{
		bodies_.addTail(elem);
	}

	void placeBodies()
	{
		TElement* elem = bodies_.first();
		while (elem)
		{
			TElement* next = elem->next();
			place(elem);
			elem = next;
		}
		flush();
	}

private:
	static const int DEFERRED_QUEUESIZE = 32;

	void put(TElement* elem, int pos)
	{
		elem->setLocation(pos);
		pos_ = pos + elem->size();
		placed_.addTail(elem);
	}

	TileKit& tile_;
	LinkedQueue<TElement> placed_;
	LinkedQueue<TElement> bodies_;
	FixedQueue<TElement*, DEFERRED_QUEUESIZE> deferred_;
	int32_t pos_;
};
