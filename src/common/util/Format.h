// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cstdarg>
#include <cstdint>
#include <cstdio>

namespace Format
{
    inline int unsafe(char* str, const char* format, ...) 
    {
        va_list args;
        va_start(args, format);

        // Disable warnings for GCC and Clang
        #if defined(__GNUC__) || defined(__clang__)
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wformat-security"
        #endif
        
        // Disable warnings for MSVC
        #ifdef _MSC_VER
        #pragma warning(push)
        #pragma warning(disable : 4996) // Disable specific warning for deprecated functions
        #endif

        int result = vsprintf(str, format, args);

        // Re-enable warnings for GCC and Clang
        #if defined(__GNUC__) || defined(__clang__)
        #pragma GCC diagnostic pop
        #endif

        // Re-enable warnings for MSVC
        #ifdef _MSC_VER
        #pragma warning(pop)
        #endif

        va_end(args);
        return result;
    }
}