#include "StringManager.h"
#include "BuildSettings.h"
#include "StringStatistics.h"

StringManager::StringManager()
{

}


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
 *     - add rest of string
 *   - b
 */

void StringManager::build(const BuildSettings& setings, const StringStatistics& strings)
{
	uint32_t protoStringCount;
	uint32_t protoKeyCount = 0;
	uint32_t protoValueCount = 0;
	uint32_t totalEntrySize = 0;
	StringStatistics::Iterator iter = strings.iter();

	for (;;)
	{
		const StringStatistics::Counter* counter = iter.next();
		if (!counter) break;

		uint32_t stringSize = counter->stringSize();
	}
}
