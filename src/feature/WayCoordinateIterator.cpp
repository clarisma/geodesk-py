// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "feature/WayCoordinateIterator.h"

WayCoordinateIterator::WayCoordinateIterator(WayPtr way)
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

void WayCoordinateIterator::start(const FeaturePtr way, int flags)
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
