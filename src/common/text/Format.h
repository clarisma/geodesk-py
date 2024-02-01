// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cstdint>
#include <iostream>
#include <sstream>
#include <thread>
#include <string>
#include <stdarg.h>

namespace Format
{
	inline std::string format(std::thread::id id)
	{
        std::stringstream ss;
        ss << id;
        return ss.str();
	}

    template <size_t N>
    inline void formatBuf(char(&buf)[N], const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        vsnprintf(buf, N, format, args);
        va_end(args);
    }

    inline void unsafe(char* buf, const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        vsprintf(buf, format, args);
        va_end(args);
    }

    inline std::string format(const char* format, ...)
    {
        char buf[1024];
        va_list args;
        va_start(args, format);
        vsnprintf(buf, sizeof(buf), format, args);
        va_end(args);
        return std::string(buf);
    }
}

