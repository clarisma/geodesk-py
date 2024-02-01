// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <common/util/varint.h>

namespace protobuf 
{
	enum Type
	{
		VARINT = 0,
		FIXED64 = 1,
		STRING = 2,
		FIXED32 = 5
	};

	struct Message
	{
		Message() : start(nullptr), end(nullptr) {}
		Message(const uint8_t* start_, const uint8_t* end_) : 
			start(start_), end(end_) {}

		bool isEmpty() const { return start == end; }
		size_t length() const { return end - start; }

		const uint8_t* start;
		const uint8_t* end;
	};

	using Field = uint32_t;

	inline Message readMessage(const uint8_t*& p)
	{
		uint32_t size = readVarint32(p);
		const uint8_t* start = p;
		p += size;
		return Message(start, p);
	}

	inline Field readField(const uint8_t*& p)
	{
		return readVarint32(p);
	}

	inline void skipEntity(const uint8_t*& p, Field field)
	{
		switch (field & 7)
		{
		case Type::VARINT:
			readVarint64(p);
			break;
		case Type::FIXED64:
			p += 8;
			break;
		case Type::STRING:
			readMessage(p);
			break;
		case Type::FIXED32:
			p += 4;
			break;
		default:
			// TODO: throw new PbfException("Unknown type: " + (marker & 7));
			break;
		}
	}

} // end namespace

