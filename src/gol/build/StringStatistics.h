// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cstdint>
#include <common/util/protobuf.h>

struct StringCounter;

struct StringCounterHeader
{
	StringCounter* next;
	uint64_t count;
	uint32_t hash;
};

struct StringCounter : public StringCounterHeader
{
	uint8_t bytes[1];
};


class StringBin
{
private:
	uint8_t* start_;
	uint8_t* end_;
};


class StringStatistics
{
public:
	StringStatistics(uint32_t tableSize, uint32_t heapSize);
	~StringStatistics();

	bool addString(const uint8_t* bytes, uint64_t count);
	bool addString(const StringCounter *pCounter);
	void removeStrings(uint32_t minCount);
	protobuf::Message takeStrings();

private:
	bool addString(const uint8_t* bytes, uint32_t size, uint32_t hash, uint64_t count);
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

	StringCounter** table_;
	uint8_t* heap_;
	uint8_t* heapEnd_;
	uint8_t* p_;
	size_t tableSize_;
	size_t countersSize_;
};
