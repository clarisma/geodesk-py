// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#define DECLARE_EXCEPTION(EXCEPTION_NAME) \
class EXCEPTION_NAME : public std::runtime_error \
{ \
public: \
    explicit EXCEPTION_NAME(const char* message) \
        : std::runtime_error(message) {} \
    explicit EXCEPTION_NAME(const std::string& message) \
        : std::runtime_error(message) {} \
    template <typename... Args> \
    explicit EXCEPTION_NAME(const char* message, Args... args) \
        : std::runtime_error(Format::format(message, args...)) {} \
};

#define DECLARE_EXCEPTION_WITH_BASE(EXCEPTION_NAME, BASE_CLASS) \
class EXCEPTION_NAME : public BASE_CLASS \
{ \
public: \
    explicit EXCEPTION_NAME(const char* message) \
        : BASE_CLASS(message) {} \
    explicit EXCEPTION_NAME(const std::string& message) \
        : BASE_CLASS(message) {} \
    template <typename... Args> \
    explicit EXCEPTION_NAME(const char* message, Args... args) \
        : BASE_CLASS(Format::format(message, args...)) {} \
};
