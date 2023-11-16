#include "Layout.h"

Layout::Layout() : 
	pos_(0)
{
}

void Layout::place(TElement* elem)
{
    int pos = elem->alignedLocation(pos_);
    if (pos != pos_)
    {
        // Element cannot be placed without forming a gap
        if (deferred_.isFull())
        {
            // The queue is full, so we force-place the oldest element
            // before adding the new element to the end
            put(deferred_.remove());
            deferred_.add(elem);
        }
        else
        {
            // There's room in the queue, so we'll defer the element
            // We return because nothing has changed, no need to 
            // attempt to place the deferred elements
            deferred_.add(elem);
            return;
        }
    }
    else
    {
        // The element can be placed without a gap
        put(elem, pos);
    }

    // Since an element has been placed (and the alignment situation
    // may have changed), let's attempt to place the deferred elements
    while (!deferred_.isEmpty())
    {
        if (deferred_.peek()->alignedLocation(pos_) != pos_) break;
        put(deferred_.remove(), pos_);
    }
}

void Layout::flush()
{
    while (!deferred_.isEmpty()) put(deferred_.remove());
}
