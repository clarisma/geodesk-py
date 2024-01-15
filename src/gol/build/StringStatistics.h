// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cstdint>
#include <memory>
#include <common/util/protobuf.h>

struct StringCounter;


/*
class StringBin
{
private:
	uint8_t* start_;
	uint8_t* end_;
};
*/

class StringStatistics
{
public:
	using StringCount = int64_t;   // Signed to allow -1 as a marker

	StringStatistics(uint32_t tableSize, uint32_t arenaSize);

	bool addString(const uint8_t* bytes, StringCount count);
	bool addString(const StringCounter *pCounter);
	void removeStrings(uint32_t minCount);
	protobuf::Message takeStrings();

private:
	using CounterOfs = uint32_t;

	struct CounterHeader
	{
		CounterOfs next;				   // offset of next counter	
		uint32_t hash;
		StringCount keys;
		StringCount values;
	};

	struct Counter : public CounterHeader
	{
		uint8_t bytes[1];
	};

	Counter* counterAt(CounterOfs ofs) const
	{
		return reinterpret_cast<Counter*>(arena_.get() + ofs);
	}
	bool addString(const uint8_t* bytes, uint32_t size, uint32_t hash, 
		StringCount keys, StringCount values);
	bool addString(const uint8_t* bytes, StringCount keys, StringCount values);
	void clearTable();
	static uint32_t stringCharCount(const uint8_t* bytes)
	{
		uint8_t len = *bytes;
		return (len & 0x80) ? ((static_cast<uint32_t>(*(bytes + 1)) << 7) | (len & 0x7f)) : len;
	}

	static uint32_t stringSize(const uint8_t* bytes)
	{
		uint8_t len = *bytes;
		return (len & 0x80) ?
			(((static_cast<uint32_t>(*(bytes + 1)) << 7) |
				(len & 0x7f)) + 2) : (len + 1);
	}

	/*
	static uint32_t hashString(const uint8_t* p, uint32_t size)
	{
		uint32_t hash = 0;
		const uint8_t* end = p + size;
		do
		{
			hash = hash * 31 + *p++;
		}
		while (p < end);
		return hash;
	}
	*/

	std::unique_ptr<CounterOfs[]> table_;
	std::unique_ptr<uint8_t[]> arena_;
	const uint8_t* arenaEnd_;
	uint8_t* p_;
	size_t tableSize_;
};
