// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
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
    inline void max(int64_t value, int64_t max)
    {
        if (value > max)
        {
            throw ValueException(Format::format("Exceeds maximum value (%d)", max));
        }
    }
}
