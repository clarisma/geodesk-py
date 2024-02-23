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
	const ShortVarString* str, uint32_t hash)
{
	uint32_t slot = hash % tableSize_;
	CounterOfs counterOfs = table_[slot];
	while (counterOfs)
	{
		Counter* pCounter = counterAt(counterOfs);
		if (pCounter->hash == hash)
		{
			/*
			// TODO: check this for possible overrun
			// but if mismatch of len, should terminate early?
			// No, memcmp() may read entire words
			if (memcmp(&pCounter->bytes, bytes, size) == 0)
			{
				return counterOfs;
			}
			*/
			if (pCounter->string == *str) return counterOfs;
		}
		counterOfs = pCounter->next;
	}
	uint32_t counterSize = Counter::grossSize(str->totalSize());
	if (p_ + counterSize > arenaEnd_) return 0;
	Counter* pCounter = reinterpret_cast<Counter*>(p_);
	new(pCounter) Counter(table_[slot], hash, str);
	CounterOfs ofs = p_ - arena_.get();
	table_[slot] = ofs;
	p_ += counterSize;
	counterCount_++;
	return ofs;
}

StringStatistics::CounterOfs StringStatistics::getCounter(const ShortVarString* str)
{
	uint32_t hash = Strings::hash(str->data(), str->length());
	return getCounter(str, hash);
}


StringStatistics::CounterOfs StringStatistics::addString(const Counter* pCounter)
{
	CounterOfs ofs = getCounter(&pCounter->string, pCounter->hash);
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
	// check();
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
		uint32_t strSize = pCounter->string.totalSize();
		uint32_t counterSize = Counter::grossSize(strSize);
		if (pCounter->total() >= minCount)
		{
			// If the string counter meets the minimum requirement,
			// we'll keep it and re-index it

			/*
			if (strSize > 255) printf("--> Keeping long string: %d bytes.\n", strSize);

			if (pCounter->total() > 1'000'000'000)
			{
				printf("!!!!! Problem !!!!\n");
			}
			*/

			uint32_t slot = pCounter->hash % tableSize_;
			pCounter->next = table_[slot];
			/*
			if(pDest != pSource) printf("Moving %d bytes from from %p to %p, skipping %d\n", 
				counterSize, pSource, pDest, (uint32_t)(pSource - pDest));
			*/
			memmove(pDest, pSource, counterSize);

			/*
			if (((Counter*)pDest)->total() > 1'000'000'000)
			{
				printf("!!!!! Problem after move !!!!\n");
			}
			*/
			table_[slot] = pDest - arena_.get();
			pDest += counterSize;
			counterCount_++;
		}
		else
		{
			// if (strSize > 255) printf("--> Evicting long string: %d bytes.\n", strSize);
			evictionCount++;
		}
		totalCount++;
		pSource += counterSize;
	}
	p_ = pDest;

	printf("Evicted %llu of %llu strings; minCount is now %d\n", 
		evictionCount, totalCount, minCount);
	// check();
}


/*
void StringStatistics::check() const
{
	Iterator it = iter();
	for (;;)
	{
		const Counter* p = it.next();
		if (!p) break;
		if (p->total() > 1'000'000'000)
		{
			printf("!!! Checked; possible problem !!!\n");
			return;
		}
	}
	printf("--- Checked; all OK ---\n");
}
*/