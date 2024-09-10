// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "ComboFilter.h"

// Filter combination rules:
// - Combined filter is FAST_TILE_FILTER if *either* is FAST_TILE_FILTER
// - Combined filter is USES_BBOX if *either* USES_BBOX
// - Combined filter is STRICT_BBOX if *both* are STRICT_BBOX
// - If combined filter is USES_BBOX:
//   - If STRICT_BBOX, use intersection of all bboxes
//   - If not STRICT_BBOX, use smallest bbox
// -  

ComboFilter::ComboFilter(const Filter* a, const Filter* b)
{
    int aFlags = a->flags();
    int bFlags = b->flags();
    int totalCount = 0;
    int fastTileFlag = (aFlags | bFlags) & FilterFlags::FAST_TILE_FILTER;
    int usesBoundsFlag = (aFlags | bFlags) & FilterFlags::USES_BBOX;
    int strictBoundsFlag = a->flags() & b->flags() & FilterFlags::STRICT_BBOX;
    acceptedTypes_ = a->acceptedTypes() & b->acceptedTypes();
    flags_ = fastTileFlag | usesBoundsFlag | strictBoundsFlag | FilterFlags::COMBO_FILTER;
    if (strictBoundsFlag)
    {
        const Box& aBounds = reinterpret_cast<const SpatialFilter*>(a)->bounds();
        const Box& bBounds = reinterpret_cast<const SpatialFilter*>(a)->bounds();
        bounds_ = Box::simpleIntersection(aBounds, bBounds);
        if (bounds_.isEmpty()) acceptedTypes_ = 0;
        // If the strict bound shave no overlap, the filter will never match anything
    }
    else
    {
        bounds_ = Box::simpleSmaller(a->getBounds(), b->getBounds());
    }
    add(a);
    add(b);
}


void ComboFilter::add(const Filter* f)
{
    if (f->isCombo())
    {
        const ComboFilter* combo = reinterpret_cast<const ComboFilter*>(f);
        for (auto it = combo->filters_.begin(); it != combo->filters_.end(); ++it)
        {
            (*it)->addref();
            filters_.push_back(*it);
        }
    }
    else
    {
        f->addref();
        filters_.push_back(f);
    }
}


ComboFilter::~ComboFilter()
{
    for (auto it = filters_.begin(); it != filters_.end(); ++it)
    {
        (*it)->release();
    }
}


bool ComboFilter::accept(FeatureStore* store, FeaturePtr feature, FastFilterHint fast) const
{
    for (auto it = filters_.begin(); it != filters_.end(); ++it)
    {
        FastFilterHint fastChild = fast;
        fastChild.turboFlags &= 1;      // mask of the turbo flag for this child
        if (!(*it)->accept(store, feature, fastChild)) return false;
        fast.turboFlags >>= 1;          // bring the next turbo flag to bit position 0
    }
    return true;
}

int ComboFilter::acceptTile(Tile tile) const
{
    int fast = 0;
    int nextFlag = 1;
    for (auto it = filters_.begin(); it != filters_.end(); ++it)
    {
        int accept = (*it)->acceptTile(tile);
        if (accept < 0) return accept;
        fast |= accept == 0 ? 0 : nextFlag;
        nextFlag <<= 1;
    }
    return fast;
}