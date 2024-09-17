// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cassert>
#include <common/math/Decimal.h>

/// Constants and functions for processing tag values.
///
/// @ingroup lowlevel
///
class TagValues
{
public:
    enum Type
    {
        NARROW_NUMBER = 0,
        GLOBAL_STRING = 1,
        WIDE_NUMBER = 2,
        LOCAL_STRING = 3
    };

    static const int MAX_COMMON_KEY = (1 << 13) - 2;
	static const int MIN_NUMBER = -256;
    static const int MAX_WIDE_NUMBER = (1 << 30) - 1 + MIN_NUMBER;
    static const int MAX_NARROW_NUMBER = (1 << 16) - 1 + MIN_NUMBER;

    // TODO: This value will change in v2!
    static constexpr uint32_t EMPTY_TABLE_MARKER = 0xffff'ffff;
    static const uint32_t EMPTY_TABLE_STRUCT[2];

    static const double SCALE_FACTORS[];

    static int32_t intFromNarrowNumber(uint_fast32_t val)
    {
        assert(val <= 0xffff);
        return static_cast<int32_t>(val) + MIN_NUMBER;
    }

    static double doubleFromWideNumber(uint32_t val)
    {
        double mantissa = (int32_t)(val >> 2) + MIN_NUMBER;
        int scale = val & 3;
        return mantissa * SCALE_FACTORS[scale];
    }

    static Decimal decimalFromWideNumber(uint32_t val)
    {
        int64_t mantissa = static_cast<int64_t>(val >> 2) + MIN_NUMBER;
        int scale = val & 3;
        return Decimal(mantissa, scale);
    }

    static bool isNumericValue(Decimal d)
    {
        if (!d.isValid()) return false;
        if (d.scale() > 3) return false;
        int64_t mantissa = d.mantissa();
        return mantissa >= MIN_NUMBER && mantissa <= MAX_WIDE_NUMBER;
    }

    static bool isNarrowNumericValue(Decimal d)
    {
        if (!d.isValid()) return false;
        if (d.scale() > 0) return false;
        int64_t mantissa = d.mantissa();
        return mantissa >= MIN_NUMBER && mantissa <= MAX_NARROW_NUMBER;
    }

    static uint32_t narrowNumber(Decimal d)
    {
        assert(isNarrowNumericValue(d));
        return static_cast<uint32_t>(d.mantissa() - MIN_NUMBER);
    }

    static uint32_t wideNumber(Decimal d)
    {
        assert(isNumericValue(d));
        return static_cast<uint32_t>(
            ((d.mantissa() - MIN_NUMBER) << 2) | d.scale());
    }
};

