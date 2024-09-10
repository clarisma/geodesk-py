// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <stdexcept>
#include <common/text/Format.h>

namespace geodesk {

class QueryException : public std::runtime_error
{
public:
    explicit QueryException(const char* message)
        : std::runtime_error(message) {}

    explicit QueryException(const std::string& message)
        : std::runtime_error(message) {}

    template <typename... Args>
    explicit QueryException(const char* message, Args... args)
        : std::runtime_error(Format::format(message, args...)) {}
};

} // namespace geodesk::detail