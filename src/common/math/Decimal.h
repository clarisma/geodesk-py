// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "Math.h"
#include <limits>
#include <common/text/Format.h>

class Decimal
{
public:
	Decimal(const char* s, bool strict = false) :
		value_(parse(s, strict))
	{
	}

	bool isValid() const noexcept { return value_ != INVALID;  }

	constexpr int64_t mantissa() const noexcept
	{
		return value_ >> 4;
	}

	constexpr int scale() const noexcept
	{
		return static_cast<int>(value_ & 15);
	}

	operator int64_t() const noexcept
	{
		if (value_ == INVALID) return value_;
		if (scale() == 0) return mantissa();
		return mantissa() / static_cast<int64_t>(Math::POWERS_OF_10[scale()]);
	}

	operator double() const noexcept 
	{
		if (value_ == INVALID) return std::numeric_limits<double>::quiet_NaN();
		double m = static_cast<double>(mantissa());
		if (scale() == 0) return m;
		return m / Math::POWERS_OF_10[scale()];
	}

	operator std::string() const
	{
		char buf[32];
		format(buf);
		return std::string(buf);
	}


	char* format(char* buf) const noexcept
	{
		if (value_ == INVALID)
		{
			memcpy(buf, "invalid", 8);
			return buf + 7;
		}

		char temp[32];
		char* end = temp + sizeof(temp);
		char* start = Format::integerReverse(value_, end);
		size_t len = end - start;
		int scale = this->scale();
		if (scale == 0)
		{
			memcpy(buf, start, len);
		}
		else
		{
			int wholePartLen = len - scale;
			memcpy(buf, start, wholePartLen);
			buf[wholePartLen] = '.';
			memcpy(buf + wholePartLen + 1, start + wholePartLen, len - wholePartLen);
			len++;
		}
		buf[len] = 0;
		return buf + len;
	}

private:
	static int64_t parse(const char* s, bool strict)
	{
		int64_t value = 0;
		int scale = 0;
		bool seenDigit = false;
		bool seenNonZeroDigit = false;
		bool seenDot = false;
		bool negative = false;
		const char* p = s;
		
		for (;;)
		{
			char ch = *p++;
			if (ch == 0) break;
			if (ch == '-')
			{
				if (p != s) return INVALID;
				negative = true;
				continue;
			}
			if (ch == '0')
			{
				if (*p == 0) return 0;
				if (strict && seenDigit && !seenNonZeroDigit) return INVALID;
				value *= 10;
				seenDigit = true;
				if (seenDot) scale++;
				continue;
			}
			if (ch == '.')
			{
				if (seenDot) return INVALID;
				if (strict && !seenDigit) return INVALID;
				seenDot = true;
				continue;
			}
			if (ch < '0' || ch > '9') return INVALID;
			seenDigit = true;
			seenNonZeroDigit = true;
			value = value * 10 + (ch - '0');
			if ((value & 0xf800'0000'0000'0000ULL) != 0) return INVALID;
			if (seenDot) scale++;
		}
		if (value == 0)
		{
			if (seenDot && !seenDigit) return INVALID;
			if (strict)
			{
				if (negative || scale == 0) return INVALID;
			}
		}
		if (strict && seenDot && scale == 0) return INVALID;
		if (scale > 15) return INVALID;
		return ((negative ? -value : value) << 4) | scale;
	}

	static constexpr int64_t INVALID = INT64_MIN;

	int64_t value_;
};