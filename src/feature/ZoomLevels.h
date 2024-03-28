// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <common/util/BitIterator.h>
#include <common/util/Bits.h>
#include <common/validate/Validate.h>

class ZoomLevels
{
public:
    ZoomLevels() : levels_(0) {}
    ZoomLevels(uint32_t levels) : levels_(levels) {}

    static const uint32_t DEFAULT = 0b1010101010101;
    using Iterator = BitIterator<uint32_t>;

    inline int count() const noexcept
    {
        return Bits::bitCount(levels_);
    }

    Iterator iter() const noexcept
    {
        return Iterator(levels_);
    }

    bool isValidZoomLevel(int zoom) const noexcept
    {
        return (levels_ & (1 << zoom)) != 0;
    }

    void check() const
    {
        if (count() > 8)
        {
            throw ValueException("Maximum 8 zoom levels");
        }

        if ((levels_ & 1) == 0)
        {
            throw ValueException("Must include root zoom level (0)");
        }
        
        uint32_t v = levels_;
        while (v)
        {
            int skip = Bits::countTrailingZerosInNonZero(v);
            if (skip > 2)
            {
                throw ValueException("Must not skip more than 2 levels");
            }
            v >>= (skip + 1);
        }
    }

    /**
     * Returns the number of levels that are skipped following the given
     * level (0, 1 or 2 in a valid tile pyramid), or -1 for the highest 
     * zoom level (which by definition does not have child levels).
     */
    int skippedAfterLevel(int zoom) const noexcept
    {
        uint32_t childLevels = levels_ >> (zoom + 1);
        return childLevels ? Bits::countTrailingZerosInNonZero(childLevels) : -1;
    }

    /**
     * Returns the highest zoom level below the given zoom level.
     * If zoom is 0, returns 0.
     */
    int parentZoom(int zoom) const noexcept
    {
        uint32_t mask = (1 << zoom) - 1;
        return 31 - Bits::countLeadingZerosInNonZero32((levels_ & mask) | 1);
    }

private:
    uint32_t levels_;
};