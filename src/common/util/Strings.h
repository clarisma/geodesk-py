// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cstdint>

namespace Strings
{
    inline size_t hash(const char* str, size_t length)
    {
        size_t hash = 5381;  // Initial value for djb2 algorithm
        for (size_t i = 0; i < length; ++i)
        {
            // hash * 33 + c
            hash = ((hash << 5) + hash) + static_cast<unsigned char>(str[i]);
        }
        return hash;
    }
}