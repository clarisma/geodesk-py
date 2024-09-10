// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "Math.h"
#include <limits>

double Math::POWERS_OF_10[] =
{
    1.0,
    10.0,
    100.0,
    1000.0,
    10000.0,
    100000.0,
    1000000.0,
    10000000.0,
    100000000.0,
    1000000000.0,
    10000000000.0,
    100000000000.0,
    1000000000000.0,
    10000000000000.0,
    100000000000000.0,
    1000000000000000.0,
};	// 15 digits max


bool Math::parseDouble(const char* s, size_t len, double* pResult)
{
    const char* end = s + len;
    // skip leading whitespace
    for (; s < end; s++)
    {
        if (*s > 32)
        {
            bool negative = false;
            bool seenDigit = false;
            const char* decimal = nullptr;
            double value = 0;

            if (*s == '-')
            {
                negative = true;
                s++;
            }
            for (; s < end; s++)
            {
                char ch = *s;
                if (ch >= '0' && ch <= '9')
                {
                    value = value * 10 + (ch - '0');
                    seenDigit = true;
                    continue;
                }
                if (ch == '.')
                {
                    if (decimal) break;
                    decimal = s + 1;
                    continue;
                }
                break;
            }
            if (seenDigit)
            {
                value = negative ? (-value) : value;
                *pResult = value / POWERS_OF_10[decimal ? (s - decimal) : 0];
                // TODO: use multiplication (factors) instead? Faster?
                return true;
            }
            break;
        }
    }
    *pResult = std::numeric_limits<double>::quiet_NaN();
    return false;
}
