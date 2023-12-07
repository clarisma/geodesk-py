// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "Way.h"
#include "FeatureStore.h"
#include "filter/Filter.h"
#include <common/util/log.h>
#include <common/util/varint.h>

uint32_t WayRef::nodeCount() const
{
    const uint8_t* p = bodyptr();
    return readVarint32(p);
}

WayCoordinateIterator::WayCoordinateIterator(WayRef way)
{
    start(way, way.flags());
}

void WayCoordinateIterator::start(const uint8_t* p, int32_t prevX, int32_t prevY, bool duplicateFirst)
{
    p_ = p;
	remaining_ = readVarint32(p_);
    duplicateFirst_ = duplicateFirst;
	x_ = prevX + readSignedVarint32(p_);
	y_ = prevY + readSignedVarint32(p_);
	firstX_ = duplicateFirst ? x_ : 0;
	firstY_ = duplicateFirst ? y_ : 0;
}

void WayCoordinateIterator::start(const FeatureRef way, int flags)
{
    start(way.bodyptr(), way.minX(), way.minY(), flags & FeatureFlags::AREA);
}

Coordinate WayCoordinateIterator::next()
{
    Coordinate c(x_, y_);
    if (--remaining_ > 0)
	{
		x_ += readSignedVarint32(p_);
		y_ += readSignedVarint32(p_);
        // assert(x_ != 0 || y_ != 0);
        // TODO: assert fails for Placeholder ways since their coords are 0/0
	}
    else
    {
        x_ = firstX_;
        y_ = firstY_;
        firstX_ = 0;
        firstY_ = 0;

        // TODO: check this
        // duplicateFirst_ = false;    // so coordinatesRemaining will be correct
    }
    return c;
}


FeatureNodeIterator::FeatureNodeIterator(FeatureStore* store)
  : store_(store),
    currentTip_(FeatureConstants::START_TIP)
{
}


void FeatureNodeIterator::start(pointer pBody, int flags, 
    const MatcherHolder* matcher, const Filter* filter) 
{
    matcher_ = matcher;
    filter_ = filter;
    currentTip_ = FeatureConstants::START_TIP;
    pForeignTile_ = nullptr;
    p_ = pBody - (flags & FeatureFlags::RELATION_MEMBER);
    currentNode_ = (flags & FeatureFlags::WAYNODE) ? 0 : MemberFlags::LAST;
}


NodeRef FeatureNodeIterator::next()
{
    while ((currentNode_ & MemberFlags::LAST) == 0)
    {
        p_ -= 4;
        pointer pCurrent = p_;
        currentNode_ = p_.getUnalignedInt();
        NodeRef feature(nullptr);
        if (currentNode_ & MemberFlags::FOREIGN)
        {
            if (currentNode_ & MemberFlags::DIFFERENT_TILE)
            {
                p_ -= 2;
                int32_t tipDelta = p_.getShort();
                if (tipDelta & 1)
                {
                    // wide TIP delta
                    p_ -= 2;
                    tipDelta = (tipDelta & 0xffff) |
                        (static_cast<int32_t>(p_.getShort()) << 16);
                }
                tipDelta >>= 1;     // signed
                currentTip_ += tipDelta;
                pForeignTile_ = store_->fetchTile(currentTip_);
            }
            feature = NodeRef(pForeignTile_ + ((currentNode_ & 0xffff'fff0) >> 2));
        }
        else
        {
            feature = NodeRef(pCurrent + (
                static_cast<int32_t>(currentNode_ & 0xffff'fffc) >> 1));
            // TODO: This may be wrong, pCurrent must be 4-byte aligned
            // In Java: pNode = (pCurrent & 0xffff_fffe) + ((node >> 2) << 1);
        }

        if(matcher_->mainMatcher().accept(feature.ptr()))
        {
            // LOG("Node matched");
            if (filter_ == nullptr || filter_->accept(store_, feature, FastFilterHint()))
            {
                // Feature accepted by matcher_ and filter_
                return feature;
            }
        }
        else
        {
            // LOG("Node NOT matched");
        }
    }
    return NodeRef(nullptr);
}


/*
WayNodeIterator::WayNodeIterator(FeatureStore* store, pointer pWay) // TODO: Filter
    : nextFeatureNode_(nullptr)
{
    int flags = pWay.getInt();
    pointer pBody = pWay.follow(12);
    int32_t minX = pWay.getInt(-16);
    int32_t minY = pWay.getInt(-12);
    new(&coords_)WayCoordinateIterator(pBody, minX, minY,
        flags & FeatureFlags::AREA);
    if (flags & FeatureFlags::WAYNODE)
    {
        new(&featureNodes_)FeatureNodeIterator(store, pBody,
            flags, nullptr);
        // TODO: "accept-all" matcher; filter
        nextFeatureNode_ = featureNodes_.next();
    }
}

*/

// TODO: check this method
bool WayRef::containsPointFast(double cx, double cy)
{
    // assert(isArea());    // TODO: don't assert, can use a series of linestrings
    WayCoordinateIterator iter;
    iter.start(*this, FeatureFlags::AREA);
    Coordinate c = iter.next();
    double x1 = c.x;
    double y1 = c.y;
    int odd = 0;
    for (;;)
    {
        c = iter.next();
        if (c.isNull()) break;
        double x2 = c.x;
        double y2 = c.y;
        
        // Skips vertex check for ~20% speedup

        if (((y1 <= cy) && (y2 > cy))     // upward crossing
            || ((y1 > cy) && (y2 <= cy))) // downward crossing
        {
            // compute edge-ray intersect x-coordinate
            double vt = (cy - y1) / (y2 - y1);
            if (cx < x1 + vt * (x2 - x1)) // P.x < intersect
            {
                odd ^= 1;
                LOG("crossing; odd is now %d", odd);
            }
        }
        x1 = x2;
        y1 = y2;
    }
    LOG("return odd = %d", odd);
    return odd;
}
