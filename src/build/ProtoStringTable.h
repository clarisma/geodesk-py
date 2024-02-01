// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cstdint>
#include <common/util/BufferWriter.h>

enum class ProtoStringType
{
	KEY = 0,
	VALUE = 1
};

/**
 * A pair of pre-encoded varints for a single proto-string.
 * To avoid excuting the branchy varint encoding instructions every time 
 * we need to write a string reference, we pre-encode the bit patterns.
 * The lowest 2 bits of each 32-bit value signify the number of bytes
 * in the varint:
 *   0 = 1-byte varint
 *   1 = 2-byte varint
 *   2 = 3-byte varint
 *   3 = 4-byte varint
 * To get the actual varint bytes, we need to right-shift the 32-bit value by 2.
 * This allows us to store references for up to 32M strings: 
 * - We need 2 bits for the byte code, which means we can store a 26-bit
 *   varint-encoded value (4 bits are needed for the varint overhad)
 *   The low-bit of the varint is used to differentiate between string-code
 *   and literal string; this leaves 25 bits = 32M possible values
 */

class ProtoStringEncoding
{
public:	
	ProtoStringEncoding(uint32_t keyCode, uint32_t valueCode)
	{
		varints_[static_cast<int>(ProtoStringType::KEY)] = encode(keyCode);
		varints_[static_cast<int>(ProtoStringType::VALUE)] = encode(valueCode);
	}

	ProtoStringEncoding(uint32_t offset)
	{
		uint32_t v = (offset << 3);
		varints_[0] = v;
		varints_[1] = v;
	}

	void writeTo(BufferWriter* writer, ProtoStringType keyOrValue)
	{
		uint32_t v = varints_[static_cast<int>(keyOrValue)];
		assert(v != 0); 
			// 0 is not a valid string reference, since it means
			// "string is not in the proto-string table"
		uint32_t len = (v & 3) + 1;		// lowest 2 bits
		v >>= 2;
		writer->writeBytes(reinterpret_cast<const char*>(&v), len);
	}

private:
	static uint32_t encode(uint32_t v)
	{
		assert(v < (1 << 25));	// we can encode maximum of 25 sig bits
		v = (v << 1) | 1;		// set the global-string marker (Bit 0)
		assert(varintSize(v) <= 4);
		uint32_t varint;
		uint8_t* p = reinterpret_cast<uint8_t*>(&varint);
		writeVarint(p, v);
		uint32_t size = static_cast<uint32_t>(p - reinterpret_cast<uint8_t*>(&varint));
		return (varint << 2) | (size - 1);
	}
	uint32_t varints_[2];
};

/*
 * A structure (one for each string in the proto-string table) that allows
 * the Compiler to turn a proto-string code into any of the following:
 *  a) a global-string code
 *  b) a local string
 *  c) a narrow number
 *  d) a wide number
 *
 * - If `keyCode`/`valuecode` is between 0 and 0xfc (inclusive), it is used
 *   as the global-string code (0 == "" and is only valid for roles);
 *   `value` is ignored (should be 0)
 * - If the code is 0xff, then `value` is interpreted as the offset to the
 *   literal string (i.e. the string is frequent enough to be in the proto-
 *   string table, which holds potentially millions of strings, but not
 *   frequent enough to be in the GST, which holds less than 64K)
 * - If the code is 0xfe, then `value` is a number in GOL notation
 *  // TODO: Do we need to distinguish wide/narrow? Can tell by magnitude of
 *     number; we can always store narrow in wide??
 *
 */

struct ProtoStringMapping
{
	uint16_t keyCode;
	uint16_t valueCode;
	uint32_t value;
};
