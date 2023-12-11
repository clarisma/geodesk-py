#include "StringStatistics.h"
#include <cstring>

StringStatistics::StringStatistics(uint32_t tableSize, uint32_t heapSize) :
	tableSize_(tableSize),
	table_(nullptr),
	heap_(nullptr)
{
	table_ = new StringCounter * [tableSize];
	clearTable();
	p_ = heap_ = new uint8_t[heapSize];
	heapEnd_ = heap_ + heapSize;
}


StringStatistics::~StringStatistics()
{
	if (table_) delete[] table_;
	if (heap_) delete[] heap_;
}

void StringStatistics::clearTable()
{
	memset(table_, 0, sizeof(StringCounter*) * tableSize_);
}


bool StringStatistics::addString(const uint8_t* bytes, uint32_t size, uint32_t hash, uint64_t count)
{
	uint32_t slot = hash % tableSize_;
	StringCounter* pCounter = table_[slot];
	while (pCounter)
	{
		if (pCounter->hash == hash)
		{
			if (memcmp(&pCounter->bytes, bytes, size) == 0)
			{
				pCounter->count += count;
				return true;
			}
		}
		pCounter = pCounter->next;
	}

	uint32_t counterSize = sizeof(StringCounterHeader) + size;
	if (p_ + counterSize > heapEnd_) return false;
	pCounter = reinterpret_cast<StringCounter*>(p_);
	pCounter->next = table_[slot];
	table_[slot] = pCounter;
	pCounter->count = count;
	pCounter->hash = hash;
	memcpy(&pCounter->bytes, bytes, size);
	p_ += (counterSize + 7) & ~7;
	return true;
}

bool StringStatistics::addString(const uint8_t* bytes, uint64_t count)
{
	uint32_t size = stringSize(bytes);
	return addString(bytes, size, hashString(bytes, size), count);
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
