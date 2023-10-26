// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cstdint>

class PbfDecoder
{
public:
	PbfDecoder(const uint8_t* p)
		: m_p(p)
	{
	}

	PbfDecoder(const char* p)
		: m_p(reinterpret_cast<const uint8_t*>(p))
	{
	}

	uint32_t readVarint32()
	{
		uint32_t val;
		uint8_t b;
		b = *m_p++;
		val = b & 0x7f;
		if ((b & 0x80) == 0) return val;
		b = *m_p++;
		val |= static_cast<uint32_t>(b & 0x7f) << 7;
		if ((b & 0x80) == 0) return val;
		b = *m_p++;
		val |= static_cast<uint32_t>(b & 0x7f) << 14;
		if ((b & 0x80) == 0) return val;
		b = *m_p++;
		val |= static_cast<uint32_t>(b & 0x7f) << 21;
		if ((b & 0x80) == 0) return val;
		b = *m_p++;
		val |= static_cast<uint32_t>(b & 0x7f) << 28;
		// if ((b & 0x80) == 0) return val;
		return val;
	}

	inline uint8_t readByte()
	{
		return  *m_p++;
	}

	inline void skip(int count)
	{
		m_p += count;
	}

	inline const uint8_t* pointer()
	{
		return m_p;
	}

private:
	const uint8_t* m_p;
};
