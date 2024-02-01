// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include "varint.h"

/**
 * A pre-encoded varint representation of a 26-bit integer.
 */
class EncodedVarint26
{
public:
	EncodedVarint26(uint32_t v)
	{
		assert(v < (1 << 26));	// we can encode maximum of 26 bits
		uint32_t varint;
		uint8_t* p = reinterpret_cast<uint8_t*>(&varint);
		writeVarint(p, v);
		uint32_t size = static_cast<uint32_t>(p - reinterpret_cast<uint8_t*>(&varint));
		value_ = (varint << 2) | (size - 1);
	}

	int size() const
	{
		return (value_ & 3) + 1;
	}

	uint32_t data() const
	{
		return value_ >> 2;
	}

private:
	uint32_t value_;
};

