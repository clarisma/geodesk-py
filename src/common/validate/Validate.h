// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <common/text/Format.h>

class ValueException : public std::runtime_error
{
public:
    explicit ValueException(const char* message)
        : std::runtime_error(message) {}

    explicit ValueException(const std::string& message)
        : std::runtime_error(message) {}
};


namespace Validate
{
    inline int64_t max(int64_t value, int64_t maxValue)
    {
        if (value > maxValue)
        {
            throw ValueException(Format::format("Exceeds maximum value (%d)", maxValue));
        }
        return value;
    }

    inline int maxInt(int64_t value, int maxValue)
    {
        return static_cast<int>(max(value, maxValue));
    }

    inline int64_t longValue(const char* s, int64_t min, int64_t max)
    {
        char* pEnd;
        int64_t v = std::strtol(s, &pEnd, 10);
        if (pEnd == s)
        {
            throw ValueException(Format::format("Expected number instead of %s", s));
        }
        if (v < min || v > max)
        {
            throw ValueException(Format::format("Must be %lld to %lld", min, max));
        }
        return v;
    }

    inline int32_t intValue(const char* s, int32_t min, int32_t max)
    {
        return static_cast<int32_t>(longValue(s, min, max));
    }
}
