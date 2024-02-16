// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <chrono>
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

    inline void timespan(char& buf, std::chrono::milliseconds ms)
    {
        if (ms < std::chrono::seconds(1)) 
        {
            unsafe(&buf, "%lldms", ms.count());
            return;
        }
        if (ms < std::chrono::minutes(1)) 
        {
            auto seconds = std::chrono::duration_cast<std::chrono::seconds>(ms);
            unsafe(&buf, "%llds %lldms", seconds.count(), (ms - seconds).count());
            return;
        }
        if (ms < std::chrono::hours(1)) {
            auto minutes = std::chrono::duration_cast<std::chrono::minutes>(ms);
            auto seconds = std::chrono::duration_cast<std::chrono::seconds>(ms - minutes);
            unsafe(&buf, "%lldm %llds", minutes.count(), seconds.count());
            return;
        }
        if (ms < std::chrono::hours(24)) 
        {
            auto hours = std::chrono::duration_cast<std::chrono::hours>(ms);
            auto minutes = std::chrono::duration_cast<std::chrono::minutes>(ms - hours);
            unsafe(&buf, "%lldh %lldm", hours.count(), minutes.count());
            return;
        }
        auto days = std::chrono::duration_cast<std::chrono::hours>(ms) / 24;
        auto hours = std::chrono::duration_cast<std::chrono::hours>(ms) % 24;
        unsafe(&buf, "%lldd %lldh", days.count(), hours.count());
    }
}

