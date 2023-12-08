// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cassert>
#include <cstdint>
#include <string_view>

inline uint32_t readVarint32(const uint8_t*& p)
{
	uint32_t val;
	uint8_t b;
	b = *p++;
	val = b & 0x7f;
	if ((b & 0x80) == 0) return val;
	b = *p++;
	val |= static_cast<uint32_t>(b & 0x7f) << 7;
	if ((b & 0x80) == 0) return val;
	b = *p++;
	val |= static_cast<uint32_t>(b & 0x7f) << 14;
	if ((b & 0x80) == 0) return val;
	b = *p++;
	val |= static_cast<uint32_t>(b & 0x7f) << 21;
	if ((b & 0x80) == 0) return val;
	b = *p++;
	val |= static_cast<uint32_t>(b & 0x7f) << 28;
	assert((b & 0x80) == 0);
	return val;
}


inline uint64_t readVarint64(const uint8_t*& p)
{
	uint64_t val;
	uint8_t b;
	b = *p++;
	val = b & 0x7f;
	if ((b & 0x80) == 0) return val;
	b = *p++;
	val |= static_cast<uint64_t>(b & 0x7f) << 7;
	if ((b & 0x80) == 0) return val;
	b = *p++;
	val |= static_cast<uint64_t>(b & 0x7f) << 14;
	if ((b & 0x80) == 0) return val;
	b = *p++;
	val |= static_cast<uint64_t>(b & 0x7f) << 21;
	if ((b & 0x80) == 0) return val;
	b = *p++;
	val |= static_cast<uint64_t>(b & 0x7f) << 28;
	if ((b & 0x80) == 0) return val;
	b = *p++;
	val |= static_cast<uint64_t>(b & 0x7f) << 35;
	if ((b & 0x80) == 0) return val;
	b = *p++;
	val |= static_cast<uint64_t>(b & 0x7f) << 42;
	if ((b & 0x80) == 0) return val;
	b = *p++;
	val |= static_cast<uint64_t>(b & 0x7f) << 49;
	if ((b & 0x80) == 0) return val;
	b = *p++;
	val |= static_cast<uint64_t>(b & 0x7f) << 56;
	if ((b & 0x80) == 0) return val;
	b = *p++;
	val |= static_cast<uint64_t>(b & 0x7f) << 63;
	assert((b & 0x80) == 0);
	return val;
}


inline int32_t readSignedVarint32(const uint8_t*& p)
{
	int32_t val = static_cast<int32_t>(readVarint32(p));
	return (val >> 1) ^ -(val & 1);
}

inline int64_t readSignedVarint64(const uint8_t*& p)
{
	int64_t val = static_cast<int64_t>(readVarint64(p));
	return (val >> 1) ^ -(val & 1);
}


inline std::string_view readStringView(const uint8_t*& p)
{
	uint32_t len = readVarint32(p);
	std::string_view sv(reinterpret_cast<const char*>(p), len);
	p += len;
	return sv;
}

inline int countVarints(const void* pStart, const void* pEnd)
{
	int count = 0;
	const char* p = reinterpret_cast<const char*>(pStart);
	while (p < pEnd)
	{
		if (*p++ >= 0) count++;
	}
	return count;
}

inline void skipVarints(const uint8_t*& p, int count)
{
	do
	{
		uint8_t b = *p++;
		count -= (b >> 7) ^ 1;
	}
	while (count);
}


inline void writeVarint(uint8_t*& p, uint64_t val)
{
	while (val >= 0x80)
	{
		*p++ = (val & 0x7f) | 0x80;
		val >>= 7;
	}
	*p++ = val;
}


inline void writeSignedVarint(uint8_t*& p, int64_t val)
{
	writeVarint(p, (val << 1) ^ (val >> 63));
}
