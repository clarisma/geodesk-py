#include "StringCatalog.h"
#include <algorithm>
#include "BuildSettings.h"
#include "StringStatistics.h"
#include "feature/types.h"

StringCatalog::StringCatalog() :
	table_(nullptr),
	tableSlotCount_(0)
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

void StringCatalog::build(const BuildSettings& settings, const StringStatistics& strings)
{
	// The minimum number of times a string must be used in order
	// to be included in the proto-string table
	uint32_t minProtoStringUsage = 100;
	// The minimum number of times a string must be used for keys or values
	// in order to be assigned a code in the proto-string table
	uint32_t minKeyValueProtoStringUsage = minProtoStringUsage / 2;
	uint32_t protoStringCount = 0;
	uint32_t protoKeyCount = 0;
	uint32_t protoValueCount = 0;
	uint32_t totalEntrySizeInBytes = 0;
	StringStatistics::Iterator iter = strings.iter();

	// First, count the number of strings that should be placed in the
	// proto-string table and measure the total space required to store them

	for (;;)
	{
		const StringStatistics::Counter* counter = iter.next();
		if (!counter) break;
		if (counter->totalCount() < minProtoStringUsage) continue;

		protoStringCount++;
		uint32_t stringSize = counter->stringSize();
		totalEntrySizeInBytes += Entry::totalSize(stringSize);
	}
	printf("Proto-string table has %d strings (%d total bytes)\n",
		protoStringCount, totalEntrySizeInBytes);

	// Allocate space for the proto-string table and copy the strings
	// into it, but don't create the actual lookup table yet
	// At the same time, create three temporary tables used to sort
	// pointers to the table entries based on the occurrence count 

	uint32_t tableSlotCount = Bytes::roundUpToPowerOf2(protoStringCount * 2);
	uint32_t tableSizeInBytes = sizeof(uint32_t) * tableSlotCount;
	uint32_t arenaSizeInBytes = tableSizeInBytes + totalEntrySizeInBytes;
	arena_.reset(new uint8_t[arenaSizeInBytes]);

	std::vector<SortEntry> sorted;
	sorted.reserve(protoStringCount);
	std::vector<SortEntry> sortedKeys;
	sortedKeys.reserve(protoStringCount);
	std::vector<SortEntry> sortedValues;
	sortedValues.reserve(protoStringCount);

	Entry* pEntry = reinterpret_cast<Entry*>(arena_.get() + tableSizeInBytes);
	iter = strings.iter();
	for (;;)
	{
		const StringStatistics::Counter* counter = iter.next();
		if (!counter) break;
		if (counter->totalCount() < minProtoStringUsage) continue;

		pEntry->string.init(&counter->string());
		pEntry->keyCode = 0;
		pEntry->valueCode = 0;
		sorted.emplace_back(counter->trueTotalCount(), pEntry);
		uint64_t keyCount = counter->keyCount();
		if (keyCount >= minKeyValueProtoStringUsage)
		{
			sortedKeys.emplace_back(keyCount, pEntry);
		}
		uint64_t valueCount = counter->valueCount();
		if (valueCount >= minKeyValueProtoStringUsage)
		{
			sortedValues.emplace_back(keyCount, pEntry);
		}
		pEntry = reinterpret_cast<Entry*>(
			reinterpret_cast<uint8_t*>(pEntry) + pEntry->totalSize());
	}

	// All strings that were counted in the first pass must be included
	// in the general sort table; for keys and values, there may be 
	// fewer strings
	assert(sorted.size() == protoStringCount);
	assert(sortedKeys.size() <= protoStringCount);
	assert(sortedValues.size() <= protoStringCount);

	sortDescending(sorted);
	sortDescending(sortedKeys);
	sortDescending(sortedValues);

	printf("Sorted strings in order of occurrence count.\n");

	// Now, create the lookup table
	// We work backwards so we index the least-used strings first; in the
	// event of a hash collision, the more frequently used string will then
	// be placed towards the head of the linked list

	memset(arena_.get(), 0, tableSizeInBytes);
	uint32_t* table = reinterpret_cast<uint32_t*>(arena_.get());
	for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) 
	{
		Entry* pEntry = it->second;
		uint32_t hash = Strings::hash(pEntry->string);
		uint32_t slot = hash % tableSlotCount;
		pEntry->next = table[slot];
		table[slot] = reinterpret_cast<uint8_t*>(pEntry) - arena_.get();
		// printf("%d: Indexed %s\n", slot, std::string(pEntry->string.toStringView()).c_str());
	}
	table_ = table;
	tableSlotCount_ = tableSlotCount;

	// Now we can build the global string table
	const std::vector<std::string_view>& indexedKeyStrings = settings.indexedKeyStrings();
	uint32_t minGlobalStringCount = CORE_STRING_COUNT + indexedKeyStrings.size();
	uint32_t maxGlobalStringCount = std::max(settings.maxStrings(), minGlobalStringCount);

	std::vector<Entry*> globalStrings;
	globalStrings.reserve(maxGlobalStringCount);
	for (int i = 0; i < StringCatalog::CORE_STRING_COUNT; i++)
	{
		addGlobalString(globalStrings, std::string_view(CORE_STRINGS[i]));
	}
	for (std::string_view str : indexedKeyStrings)
	{
		addGlobalString(globalStrings, str);
	}
	assert(globalStrings.size() <= maxGlobalStringCount);

	uint32_t minGlobalStringUsage = settings.minStringUsage();
	const static uint32_t MAX_MIXED_STRINGS = 512;
	uint32_t maxMixedStringCount = std::min(MAX_MIXED_STRINGS, maxGlobalStringCount);

	// Add the most common keys and values

	auto itSorted = sorted.begin();
	while (itSorted != sorted.end())
	{
		if (globalStrings.size() >= maxMixedStringCount) break;
		if (itSorted->first >= minGlobalStringUsage)
		{
			addGlobalString(globalStrings, itSorted->second);
		}
		itSorted++;
	}
	
	// Fill the table space up to 8K only with keys

	uint32_t maxKeyCount = std::min(1U << 13, maxGlobalStringCount);
		// TODO: use constant (max 8K keys);
	for (SortEntry entry : sortedKeys)
	{
		if (globalStrings.size() >= maxKeyCount) break;
		if (entry.first >= minGlobalStringUsage)
		{
			addGlobalString(globalStrings, entry.second);
		}
	}

	// Finally, add all remaining keys/values that fit into the table

	while (itSorted != sorted.end())
	{
		if (globalStrings.size() >= maxGlobalStringCount) break;
		if (itSorted->first >= minGlobalStringUsage)
		{
			addGlobalString(globalStrings, itSorted->second);
		}
		itSorted++;
	}

	printf("Created global string table with %lld strings.\n", globalStrings.size());
	for (Entry* entry : globalStrings)
	{
		// printf("- %s\n", std::string(entry->string.toStringView()).c_str());
	}


	// TODO: clear the markers??
}

void StringCatalog::sortDescending(std::vector<SortEntry>& sorted)
{
	std::sort(sorted.begin(), sorted.end(), [](const SortEntry& a, const SortEntry& b)
		{
			return a.first > b.first;
		});
}


StringCatalog::Entry* StringCatalog::lookup(const std::string_view str) const noexcept
{
	uint32_t slot = Strings::hash(str) % tableSlotCount_;
	printf("Looking up '%s' in slot %d...\n", std::string(str).c_str(), slot);
	uint32_t ofs = table_[slot];
	while (ofs)
	{
		Entry* p = reinterpret_cast<Entry*>(arena_.get() + ofs);
		if (p->string == str) return p;
		ofs = p->next;
	}
	return nullptr;
}

void StringCatalog::addGlobalString(std::vector<Entry*>& globalStrings, Entry* p)
{
	if (!p->keyCode)
	{
		// Strign is not already in the GST
		globalStrings.push_back(p);
		p->keyCode = globalStrings.size();
		// mark the entry with the global-string code + 1
	}
}

void StringCatalog::addGlobalString(std::vector<Entry*>& globalStrings, std::string_view str)
{
	Entry* p = lookup(str);
	assert(p); // string must exist
	addGlobalString(globalStrings, p);
}
