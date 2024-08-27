// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cassert>
#include <common/math/Decimal.h>


namespace TagValue {

static const int MIN_NUMBER = -256;
static const int MAX_WIDE_NUMBER = (1 << 30) - 1 + MIN_NUMBER;
static const int MAX_NARROW_NUMBER = (1 << 16) - 1 + MIN_NUMBER;

static const double SCALE_FACTORS[] = { 1.0, 0.1, 0.01, 0.001 };

static inline int32_t intFromNarrowNumber(uint_fast32_t val)
{
    assert(val <= 0xffff);
    return static_cast<int32_t>(val) + MIN_NUMBER;
}

static inline double doubleFromWideNumber(uint32_t val)
{
    double mantissa = (int32_t)(val >> 2) + MIN_NUMBER;
    int scale = val & 3;
    return mantissa * SCALE_FACTORS[scale];
}

static inline bool isNumericValue(Decimal d)
{
    if (!d.isValid()) return false;
    if (d.scale() > 3) return false;
    int64_t mantissa = d.mantissa();
    return mantissa >= MIN_NUMBER && mantissa <= MAX_WIDE_NUMBER;
}

static inline bool isNarrowNumericValue(Decimal d)
{
    if (!d.isValid()) return false;
    if (d.scale() > 0) return false;
    int64_t mantissa = d.mantissa();
    return mantissa >= MIN_NUMBER && mantissa <= MAX_NARROW_NUMBER;
}

} // namespace TagValue