#pragma once
#include <common/data/FixedQueue.h>
#include "TElement.h"

class Layout
{
public:
	Layout();

	int32_t size() { return pos_; }
	void place(TElement* elem);
	void flush();

	void put(TElement* elem)
	{
		put(elem, elem->alignedLocation(pos_));
	}


private:
	static const int DEFERRED_QUEUESIZE = 32;

	void put(TElement* elem, int pos)
	{
		elem->setLocation(pos);
		// TODO: chain elements?
		pos_ += elem->size();
		// TODO: set last in chain?
	}

	FixedQueue<TElement*, DEFERRED_QUEUESIZE> deferred_;
	int32_t pos_;
};
