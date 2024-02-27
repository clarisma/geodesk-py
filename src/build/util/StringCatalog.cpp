#include "StringCatalog.h"
#include <algorithm>
#include "BuildSettings.h"
#include "StringStatistics.h"

StringCatalog::StringCatalog()
{

}

const char* StringCatalog::CORE_STRINGS[] =
{
	"",
	"no",
	"yes",
	"outer",
	"inner"
};


/**
 * - Step 1: Count strings and measure required space
 * - Step 2
 *   - Build the string -> proto-code lookup, but don't fill it yet
 *   - Build a 3 sort tables (pointer to lookup entry, total usage)
 *     - keys, values, combined
 *   - Sort the tables
 * - Step 3
 *   - Build the GST
 *     - add the core strings, mark them in the proto-code lookup
 *     - add indexed keys, mark them in the proto-code lookup
 *     - add most-frequent strings, up to pos. 255, mark them
 *     - add most-frequent keys, up to pos. 8191, mark them
 *     - add rest of strings
 *   - Step 4
 *     
 */

void StringCatalog::build(const BuildSettings& setings, const StringStatistics& strings)
{
	uint32_t minProtoStringUsage = 100;
	uint32_t protoStringCount = 0;
	uint32_t protoKeyCount = 0;
	uint32_t protoValueCount = 0;
	uint32_t totalEntrySize = 0;
	StringStatistics::Iterator iter = strings.iter();

	// First, count the number of strings that should be placed in the
	// proto-string table and measure the total space required to store them

	for (;;)
	{
		const StringStatistics::Counter* counter = iter.next();
		if (!counter) break;
		if (counter->total() < minProtoStringUsage) continue;

		protoStringCount++;
		uint32_t stringSize = counter->stringSize();
		totalEntrySize += Entry::totalSize(stringSize);
	}
	printf("Proto-string table has %d strings (%d total bytes)\n",
		protoStringCount, totalEntrySize);

	// Allocate space for the proto-string table and copy the strings
	// into it, but don't create the actual lookup table yet
	// At the same time, create three temporary tables used to sort
	// pointers to the table entries based on the occurrence count 

	uint32_t tableSlotCount = Bytes::roundUpToPowerOf2(protoStringCount * 2);
	uint32_t tableSize = sizeof(uint32_t) * tableSlotCount;
	uint32_t arenaSize = tableSize + totalEntrySize;
	arena_.reset(new uint8_t[arenaSize]);
	Entry* pEntry = reinterpret_cast<Entry*>(arena_.get() + tableSize);
	SortEntry* sortArena = new SortEntry[protoStringCount * 3];
	SortEntry* pSortedEntries = sortArena;
	SortEntry* pSortedKeyEntries = sortArena + protoStringCount;
	SortEntry* pSortedValueEntries = pSortedKeyEntries + protoStringCount;
	SortEntry* pNextSortedEntry = pSortedEntries;
	SortEntry* pNextSortedKeyEntry = pSortedKeyEntries;
	SortEntry* pNextSortedValueEntry = pSortedValueEntries;

	for (;;)
	{
		const StringStatistics::Counter* counter = iter.next();
		if (!counter) break;
		if (counter->total() < minProtoStringUsage) continue;

		pEntry->string.init(&counter->string);
		pEntry->keyCode = 0;
		pEntry->valueCode = 0;
		*pNextSortedEntry++ = SortEntry(counter->total(), pEntry);
		*pNextSortedKeyEntry++ = SortEntry(counter->keyCount(), pEntry);
		*pNextSortedValueEntry++ = SortEntry(counter->valueCount(), pEntry);

		pEntry = reinterpret_cast<Entry*>(
			reinterpret_cast<uint8_t*>(pEntry) + pEntry->totalSize());
	}

	sortDescending(pSortedEntries, pSortedEntries + protoStringCount);
	sortDescending(pSortedKeyEntries, pSortedKeyEntries + protoStringCount);
	sortDescending(pSortedValueEntries, pSortedValueEntries + protoStringCount);

	printf("Sorted strings in order of occurrence count.\n");
}

void StringCatalog::sortDescending(SortEntry* start, SortEntry* end)
{
	std::sort(start, end, [](const SortEntry& a, const SortEntry& b) 
		{
			return a.first > b.first;
		});
}
