// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cassert>
#include <cstdint>
#include "Bits.h"

namespace Bytes
{
    inline uint32_t aligned(uint32_t v, uint32_t alignment)
    {
        assert(Bits::bitCount(alignment) == 1);
        return (v + alignment - 1) & ~(alignment - 1);
    }

    inline uint32_t reverseByteOrder32(uint32_t value)
    {
        return ((value & 0x000000FF) << 24) |
            ((value & 0x0000FF00) << 8) |
            ((value & 0x00FF0000) >> 8) |
            ((value & 0xFF000000) >> 24);
    }

    inline uint64_t roundUpToPowerOf2(uint64_t v)
    {
        int leadingZeroes = Bits::countLeadingZerosInNonZero64(v | 1);
        return 1ULL << (64 - leadingZeroes);
    }
}
