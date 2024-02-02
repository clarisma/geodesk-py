// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "StringStatistics.h"
#include <cstring>
#include <common/util/Strings.h>

StringStatistics::StringStatistics(uint32_t tableSize, uint32_t arenaSize) :
	tableSize_(tableSize),
	table_(new CounterOfs[tableSize]),
	arena_(new uint8_t[arenaSize]),
	counterCount_(0)
{
	reset(arenaSize);
}

void StringStatistics::reset(uint32_t arenaSize)
{
	p_ = arena_.get() + sizeof(uint32_t);
	// We use the first 4 bytes as the actual size of the arena
	// (This also allows us to use 0 as a null value, since no
	//  Counter will ever be placed at offset 0)
	arenaEnd_ = arena_.get() + arenaSize;
	clearTable();
}

void StringStatistics::clearTable()
{
	memset(table_.get(), 0, sizeof(CounterOfs) * tableSize_);
}


StringStatistics::CounterOfs StringStatistics::getCounter(
	const uint8_t* bytes, uint32_t size, uint32_t hash)
{
	uint32_t slot = hash % tableSize_;
	CounterOfs counterOfs = table_[slot];
	while (counterOfs)
	{
		Counter* pCounter = counterAt(counterOfs);
		if (pCounter->hash == hash)
		{
			// TODO: check this for possible overrun
			// but if mismatch of len, should terminate early?
			// No, memcmp() may read entire words
			if (memcmp(&pCounter->bytes, bytes, size) == 0)
			{
				return counterOfs;
			}
		}
		counterOfs = pCounter->next;
	}
	uint32_t counterSize = Counter::grossSize(size);
	if (p_ + counterSize > arenaEnd_) return 0;
	Counter* pCounter = reinterpret_cast<Counter*>(p_);
	new(pCounter) Counter(table_[slot], hash, size, bytes);
	CounterOfs ofs = p_ - arena_.get();
	table_[slot] = ofs;
	p_ += counterSize;
	counterCount_++;
	return ofs;
}

StringStatistics::CounterOfs StringStatistics::getCounter(const uint8_t* bytes)
{
	// TODO: make this more efficient!
	uint32_t len = stringCharCount(bytes);
	uint32_t size = stringSize(bytes);
	uint32_t hash = Strings::hash(
		reinterpret_cast<const char*>(bytes + size - len), len);
	return getCounter(bytes, size, hash);
}

/*
StringStatistics::CounterOfs StringStatistics::addString(
	const uint8_t* bytes, uint32_t size, uint32_t hash,
	StringCount keys, StringCount values)
{
	CounterOfs ofs = getCounter(bytes, size, hash)
	pCounter->hash = hash;
	pCounter->keys = keys;
	pCounter->values = values;
	memcpy(&pCounter->bytes, bytes, size);
	CounterOfs ofs = p_ - arena_.get();
	table_[slot] = ofs;
	p_ += (counterSize + 3) & ~3;		
		// maintain 4-byte alignment
		// TODO: Not portable since we are using 64-bit StringCount
		// Should align on 8 bytes instead?
	return ofs;
}

StringStatistics::CounterOfs StringStatistics::addString(const uint8_t* bytes, StringCount keys, StringCount values)
{
	uint32_t size = stringSize(bytes);
	return addString(bytes, size, Strings::hash(
		reinterpret_cast<const char*>(bytes), size), keys, values);
}

uint32_t hash = static_cast<uint32_t>(Strings::hash(
	reinterpret_cast<const char*>(bytes), size));
*/

StringStatistics::CounterOfs StringStatistics::addString(const Counter* pCounter)
{
	CounterOfs ofs = getCounter(pCounter->bytes, stringSize(pCounter->bytes), 
		pCounter->hash);
	if (ofs == 0) return 0;
	counterAt(ofs)->add(pCounter->keys, pCounter->values);
	return ofs;
}


std::unique_ptr<uint8_t[]> StringStatistics::takeStrings()
{
	uint32_t bytesUsed = p_ - arena_.get();
	*reinterpret_cast<uint32_t*>(arena_.get()) = bytesUsed;
	size_t arenaSize = arenaEnd_ - arena_.get();
	std::unique_ptr<uint8_t[]> oldArena(arena_.release());
	arena_.reset(new uint8_t[arenaSize]);
	reset(arenaSize);
	return oldArena;
}


void StringStatistics::removeStrings(uint32_t minCount)
{
	clearTable();
	counterCount_ = 0;
	uint8_t* pSource = arena_.get() + sizeof(uint32_t);		// skip to pos 4
	uint8_t* pDest = pSource;

	// TODO: Stop at a point before arenaEnd_ so recently added strings have a chance
	// to catch up?

	printf("Evicting strings with usage of less than %d...\n", minCount);

	uint64_t totalCount = 0;
	uint64_t evictionCount = 0;
	while (pSource < p_)
	{
		Counter* pCounter = reinterpret_cast<Counter*>(pSource);
		uint32_t counterSize = Counter::grossSize(stringSize(pCounter->bytes));
		if (pCounter->total() >= minCount)
		{
			// If the string counter meets the minimum requirement,
			// we'll keep it and re-index it

			memmove(pDest, pSource, counterSize);
			pCounter = reinterpret_cast<Counter*>(pDest);
			uint32_t slot = pCounter->hash % tableSize_;
			pCounter->next = table_[slot];
			table_[slot] = pDest - arena_.get();
			pDest += counterSize;
			counterCount_++;
		}
		else
		{
			evictionCount++;
		}
		totalCount++;
		pSource += counterSize;
	}
	p_ = pDest;

	printf("Evicted %llu of %llu strings; minCount is now %d\n", 
		evictionCount, totalCount, minCount);
}
