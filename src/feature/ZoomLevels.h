// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <common/util/Bits.h>
#include <common/validate/Validate.h>

class ZoomLevels
{
public:
    ZoomLevels(uint32_t levels) : levels_(levels) {}

    static const uint32_t DEFAULT = 0b1010101010101;

    inline int count()
    {
        return Bits::bitCount(levels_);
    }

    bool isValidZoomLevel(int zoom)
    {
        return (levels_ & (1 << zoom)) != 0;
    }

    void check()
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

private:
    uint32_t levels_;
};