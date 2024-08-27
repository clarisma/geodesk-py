// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstring>
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

    #pragma warning(push)  // Save the current warning state
    #pragma warning(disable : 4996)  // Disable C4996 warning
        // C4996 concerns potential overflow of a buffer passed to vsprintf

    inline void unsafe(char* buf, const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        vsprintf(buf, format, args);
        va_end(args);
    }

    inline void unsafe(char* buf, const char* format, va_list args)
    {
        vsprintf(buf, format, args);
    }

    #pragma warning(pop)   // Restore the previous warning state

    inline std::string format(const char* format, ...)
    {
        char buf[1024];
        va_list args;
        va_start(args, format);
        vsnprintf(buf, sizeof(buf), format, args);
        va_end(args);
        return std::string(buf);
    }

    inline void timespan(char* buf, std::chrono::milliseconds ms)
    {
        if (ms < std::chrono::seconds(1)) 
        {
            unsafe(buf, "%lldms", ms.count());
            return;
        }
        if (ms < std::chrono::minutes(1)) 
        {
            auto seconds = std::chrono::duration_cast<std::chrono::seconds>(ms);
            unsafe(buf, "%llds %lldms", seconds.count(), (ms - seconds).count());
            return;
        }
        if (ms < std::chrono::hours(1)) {
            auto minutes = std::chrono::duration_cast<std::chrono::minutes>(ms);
            auto seconds = std::chrono::duration_cast<std::chrono::seconds>(ms - minutes);
            unsafe(buf, "%lldm %llds", minutes.count(), seconds.count());
            return;
        }
        if (ms < std::chrono::hours(24)) 
        {
            auto hours = std::chrono::duration_cast<std::chrono::hours>(ms);
            auto minutes = std::chrono::duration_cast<std::chrono::minutes>(ms - hours);
            unsafe(buf, "%lldh %lldm", hours.count(), minutes.count());
            return;
        }
        auto days = std::chrono::duration_cast<std::chrono::hours>(ms) / 24;
        auto hours = std::chrono::duration_cast<std::chrono::hours>(ms) % 24;
        unsafe(buf, "%lldd %lldh", days.count(), hours.count());
    }

    /**
     * Formats time in the form HH:MM:SS or HH:MM:SS.fff, without a
     * terminating null character.
     * 
     * @param   buf  buffer with at least 8 (or 12 for ms) characters
     * @param   s    seconds since midnight (or a start time)
     *               Must be less than 24 hours (clock) or 100 hours (timer),
     *               otherwise the result will be undefined
     * @param   ms   remainder in milliseconds (0-999) or -1 to omit
     * @return  pointer to the character immediately after
     */
    inline char* timeFast(char* buf, int s, int ms)
    {
        div_t d = div(s, 60);
        int m = d.quot;
        s = d.rem;
        d = div(m, 60);
        int h = d.quot;
        m = d.rem;
        d = div(h, 10);
        buf[0] = '0' + d.quot;
        buf[1] = '0' + d.rem;
        buf[2] = ':';
        d = div(m, 10);
        buf[3] = '0' + d.quot;
        buf[4] = '0' + d.rem;
        buf[5] = ':';
        d = div(s, 10);
        buf[6] = '0' + d.quot;
        buf[7] = '0' + d.rem;
        if (ms < 0) return &buf[8];
        buf[8] = '.';
        d = div(ms, 10);
        buf[11] = '0' + d.rem;
        d = div(d.quot, 10);
        buf[10] = '0' + d.rem;
        buf[9] = '0' + d.quot;
        return &buf[12];
    }

    /**
     * Formats an unsigned integr value, placing digits from end to start.
     * 
     * @param d    unsigned integer value
     * @param end  pointer to the character that follows the last digit 
     * @return     pointer to the first digit
     */
    inline char* unsignedIntegerReverse(unsigned long long d, char* end)
    {
        do
        {
            lldiv_t result = lldiv(d, 10);
            *(--end) = static_cast<char>('0' + result.rem);
            d = result.quot;
        }
        while (d != 0);
        return end;
    }

    inline char* integerReverse(long long d, char* end)
    {
        bool negative = d < 0;
        d = negative ? -d : d;
        end = unsignedIntegerReverse(d, end);
        *(end - 1) = '-';
        return end - negative;
    }

    // TODO: standardize behavior:
    // - should format methods add zero at end?
    // - should format methods return pointer to next char?
    inline char* integer(char* p, int64_t d)
    {
        char buf[32];
        char* end = buf + sizeof(buf);
        char* start = integerReverse(d, end);
        size_t len = end - start;
        memcpy(p, start, len);
        p += len;
        *p = 0;        
        return p;
    }

    static const char* HEX_DIGITS_LOWER = "0123456789abcdef";
    static const char* HEX_DIGITS_UPPER = "0123456789ABCDEF";

    inline char* hex(char* buf, uint64_t v, int nDigits, const char* digitChars)
    {
        assert(nDigits > 0 && nDigits <= 16);
        for (int i=nDigits-1; i >= 0; i--)
        {
            buf[i] = digitChars[v & 0xf];
            v >>= 4;
        }
        buf[nDigits] = 0;
        return buf;
    }

    inline char* hex(char* buf, uint64_t v, int nDigits)
    {
        return hex(buf, v, nDigits, HEX_DIGITS_LOWER);
    }

    inline char* hexUpper(char* buf, uint64_t v, int nDigits)
    {
        return hex(buf, v, nDigits, HEX_DIGITS_UPPER);
    }
}

