// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <common/data/FixedQueue.h>
#include "TileModel.h"

class Layout
{
public:
	Layout(TileModel& tile);

	TileModel& tile() const { return tile_; }
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
		assert(elem->location() == 0);
		bodies_.addTail(elem);
		elem->setLocation(-1);
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
		assert(elem->location() <= 0);
		elem->setLocation(pos);
		pos_ = pos + elem->size();
		placed_.addTail(elem);
#ifdef _DEBUG
		count(elem);
#endif
	}

	TileModel& tile_;
	LinkedQueue<TElement> placed_;
	LinkedQueue<TElement> bodies_;
	FixedQueue<TElement*, DEFERRED_QUEUESIZE> deferred_;
	int32_t pos_;

#ifdef _DEBUG
	void count(TElement* e);
public:
	ElementCounts counts_;
#endif
};
