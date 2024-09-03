// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cstdint>

#ifdef _MSC_VER
#include <intrin.h>
#endif

#ifdef __GNUC__
#include <x86intrin.h>
#endif

namespace Bits
{
    /*
     * Counts the number of leading zeroes in i. If i is 0, the result is
     * undefined.
     */
    inline int countLeadingZerosInNonZero32(uint32_t i)
    {
#ifdef __GNUC__  // This branch also typically applies to Clang.
        return __builtin_clz(i);

#elif defined(_MSC_VER)
        unsigned long index;  // Note: 'long' in this context is always 32 bits on Windows, even in 64-bit compilations.
        _BitScanReverse(&index, i);
        return 31 - index;
#else
        // Fallback version for other compilers, not as efficient.
        int count = 0;
        for (int pos = 31; pos >= 0; pos--) 
        {
            if ((i & (1 << pos)) != 0) 
            {
                break;
            }
            count++;
        }
        return count;
#endif
    }

    inline int countLeadingZerosInNonZero64(uint64_t i)
    {
#ifdef __GNUC__  // This branch also typically applies to Clang.
        return __builtin_clzll(i);

#elif defined(_MSC_VER)
        unsigned long index;  // Note: 'long' in this context is always 32 bits on Windows, even in 64-bit compilations.
        _BitScanReverse64(&index, i);
        return 63 - index;
#else
        // Fallback version for other compilers, not as efficient.
        int count = 0;
        for (int pos = 63; pos >= 0; pos--)
        {
            if ((i & (1 << pos)) != 0)
            {
                break;
            }
            count++;
        }
        return count;
#endif
    }


    inline int countLeadingZeros32(uint32_t i)
    {
        if (i == 0) return 32;
        return countLeadingZerosInNonZero32(i);
    }

    inline int countTrailingZerosInNonZero(uint64_t n) 
    {
#if defined(__GNUC__) || defined(__clang__)  // GCC or Clang compiler
        return __builtin_ctzll(n);
#elif defined(_MSC_VER)  // MSVC compiler
        unsigned long index;
        _BitScanForward64(&index, n);
        return static_cast<int>(index);
#else
        // Fallback method for other compilers
        int count = 0;
        while ((n & 1) == 0) {
            count++;
            n >>= 1;
        }
        return count;
#endif
    }


#ifdef __GNUC__

    inline int bitCount(uint32_t x)
    {
        return __builtin_popcount(x);
    }

    inline int bitCount(uint64_t x)
    {
        return __builtin_popcountll(x);
    }

#elif defined(_MSC_VER)

    inline int bitCount(uint32_t x) 
    {
        return __popcnt(x);
    }

    inline int bitCount(uint64_t x)
    {
        return static_cast<int>(__popcnt64(x));
    }

#else

    // Fallback solution if intrinsics are not available
    inline int bitCount(uint32_t x)
    {
        x = x - ((x >> 1) & 0x55555555);
        x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
        x = (x + (x >> 4)) & 0x0F0F0F0F;
        x = x + (x >> 8);
        x = x + (x >> 16);
        return x & 0x3F;
    }

    inline int bitCount(uint64_t x)
    {
        x = x - ((x >> 1) & 0x5555555555555555);
        x = (x & 0x3333333333333333) + ((x >> 2) & 0x3333333333333333);
        x = (x + (x >> 4)) & 0x0F0F0F0F0F0F0F0F;
        x = x + (x >> 8);
        x = x + (x >> 16);
        x = x + (x >> 32);
        return x & 0x7F;
    }

#endif

    /*
    inline int64_t signedFromBit0(uint64_t val)
    {
        return static_cast<int64_t>((val >> 1) ^ -(val & 1));
    }
    */
}
