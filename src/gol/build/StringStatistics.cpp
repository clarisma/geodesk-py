// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "StringStatistics.h"
#include <cstring>
#include <common/util/Strings.h>

StringStatistics::StringStatistics(uint32_t tableSize, uint32_t arenaSize) :
	tableSize_(tableSize),
	table_(new CounterOfs[tableSize]),
	arena_(new uint8_t[arenaSize])
{
	clearTable();
	p_ = arena_.get() + sizeof(uint32_t);
		// We use the first 4 bytes as the actual size fo the arena
		// (This also allows us to use 0 as a null value, since no
		//  Counter will ever be placed at offset 0)
	arenaEnd_ = p_ + arenaSize;
}

void StringStatistics::clearTable()
{
	memset(table_.get(), 0, sizeof(CounterOfs) * tableSize_);
}


bool StringStatistics::addString(const uint8_t* bytes, uint32_t size, uint32_t hash, 
	StringCount keys, StringCount values)
{
	uint32_t slot = hash % tableSize_;
	CounterOfs counterOfs = table_[slot];
	while (counterOfs)
	{
		Counter* pCounter = counterAt(counterOfs);
		if (pCounter->hash == hash)
		{
			if (memcmp(&pCounter->bytes, bytes, size) == 0)
			{
				pCounter->keys += keys;
				pCounter->values += values;
				return true;
			}
		}
		counterOfs = pCounter->next;
	}

	uint32_t counterSize = sizeof(CounterHeader) + size;
	if (p_ + counterSize > arenaEnd_) return false;
	Counter* pCounter = reinterpret_cast<Counter*>(p_);
	pCounter->next = table_[slot];
	pCounter->hash = hash;
	pCounter->keys = keys;
	pCounter->values = values;
	memcpy(&pCounter->bytes, bytes, size);
	table_[slot] = p_ - arena_.get();
	p_ += (counterSize + 3) & ~3;		
		// maintain 4-byte alignment
		// TODO: Not portable since we are using 64-bit StringCount
		// Should align on 8 bytes instead?
	return true;
}

bool StringStatistics::addString(const uint8_t* bytes, StringCount keys, StringCount values)
{
	uint32_t size = stringSize(bytes);
	return addString(bytes, size, Strings::hash(bytes, size), count);
}

bool StringStatistics::addString(const StringCounter* pCounter)
{
	return addString(pCounter->bytes, stringSize(pCounter->bytes), 
		pCounter->hash, pCounter->count);
}


protobuf::Message StringStatistics::takeStrings()
{
	protobuf::Message msg(heap_, p_);
	size_t heapSize = heapEnd_ - heap_;
	heap_ = nullptr;	// Set to null in case alloc fails
	p_ = heap_ = new uint8_t[heapSize];
	heapEnd_ = heap_ + heapSize;
	clearTable();
	return msg;
}


void StringStatistics::removeStrings(uint32_t minCount)
{
	clearTable();
	uint8_t* pSource = heap_;
	uint8_t* pDest = heap_;

	// TODO: Stop at a point before heapEnd_ so recently added strings have a chance
	// to catch up?

	while (pSource < heapEnd_)
	{
		StringCounter* pCounter = reinterpret_cast<StringCounter*>(pSource);
		uint32_t counterSize = sizeof(StringCounterHeader) + stringSize(pCounter->bytes);
		counterSize = (counterSize + 7) & ~7;
		if (pCounter->count >= minCount)
		{
			// If the string counter meets the minimum requirement,
			// we'll keep it and re-index it

			memmove(pDest, pSource, counterSize);
			pCounter = reinterpret_cast<StringCounter*>(pDest);
			uint32_t slot = pCounter->hash % tableSize_;
			pCounter->next = table_[slot];
			table_[slot] = pCounter;
			pDest += counterSize;
		}
		pSource += counterSize;
	}
	p_ = pDest;
}
