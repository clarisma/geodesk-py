// Copyright (c) 2023 Clarisma / GeoDesk contributors
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
		Message() : p(nullptr), pEnd(nullptr) {}
		Message(const uint8_t* start, uint8_t* end) : p(start), pEnd(end) {}

		bool isEmpty() const { return p == pEnd; }

		const uint8_t* p;
		const uint8_t* pEnd;
	};

	using Field = uint32_t;

	inline Message readMessage(const uint8_t*& p)
	{
		Message msg;
		uint32_t size = readVarint32(p);
		msg.p = p;
		p += size;
		msg.pEnd = p;
		return msg;
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

