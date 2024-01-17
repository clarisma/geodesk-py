// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "Analyzer.h"
#include <string>
#include "geom/Tile.h"
#include "geom/Mercator.h"

// TODO: Need to flush remaining strings at end
// TODO: race condition

Analyzer::Analyzer(int numberOfThreads) : 
	OsmPbfReader(numberOfThreads),
	strings_(outputTableSize(), outputArenaSize()),
	minStringCount_(2),
	progress_("Analyzing")
{
}


AnalyzerContext::AnalyzerContext(Analyzer* analyzer) :
	OsmPbfContext<AnalyzerContext, Analyzer>(analyzer),
	nodeCounts_(new uint32_t[4096 * 4096]),
	strings_(analyzer->workerTableSize(), analyzer->workerArenaSize())
{
	memset(nodeCounts_.get(), 0, 4096 * 4096 * sizeof(uint32_t));
}


void AnalyzerContext::flush()
{
	LOG("== Flushing context %p with %d strings", this, strings_.counterCount());
	std::unique_ptr<uint8_t[]> strings = strings_.takeStrings();

	// Now that we've reset the String Statistics, the lookup table entries
	// are no longer valid -- we need to reset each counter offset to 0,
	// so the next lookup creates a new counter

	for (auto& entry : stringCodeLookup_) 
	{
		entry.counterOfs = 0;
	}
	 
	reader()->postOutput(AnalyzerOutputTask(strings.release(), blockBytesProcessed()));
	resetBlockBytesProcessed();
}

void AnalyzerContext::node(int64_t id, int32_t lon100nd, int32_t lat100nd, protobuf::Message& tags)
{
	// TODO: Could use a cheaper projection function since coordinates just need
	// to be approximate

	/*
	int x = Mercator::xFromLon100nd(lon100nd);
	int y = Mercator::yFromLat100nd(lat100nd);
	int col = Tile::columnFromXZ(x, 12);
	int row = Tile::rowFromYZ(y, 12);
	nodeCounts_[row * 4096 + col]++;
	*/

	const uint8_t* p = tags.start;
	while (p < tags.end)
	{
		uint32_t key = readVarint32(p);
		if (key == 0) break;
		uint32_t value = readVarint32(p);
		countString(key, 1, 0);
		countString(value, 0, 1);
		stats_.tagCount++;
	}
	tags.start = p;			
		// TODO: This feels hacky; start/end should be immutable, and node()
		// should not have the responsibility to advance start pointer.
		// However, this is the fastest approach
	stats_.nodeCount++;
	stats_.maxNodeId = id;	// assumes nodes are ordered by ID
}


void AnalyzerContext::way(int64_t id, protobuf::Message keys, protobuf::Message values, protobuf::Message nodes)
{
	countStrings(keys, 1, 0);
	stats_.tagCount += countStrings(values, 0, 1);
	stats_.wayCount++;
	stats_.maxWayId = id;  // assumes ways are ordered by ID
}

void AnalyzerContext::relation(int64_t id, protobuf::Message keys, protobuf::Message values, 
	protobuf::Message roles, protobuf::Message memberIds, protobuf::Message memberTypes)
{
	countStrings(keys, 1, 0);
	stats_.tagCount += countStrings(values, 0, 1);
	stats_.memberCount += countStrings(roles, 0, 1);
	stats_.relationCount++;
	stats_.maxRelationId = id;  // assumes relations are ordered by ID
}

void AnalyzerContext::countString(uint32_t index, int keys, int values)
{
	StringLookupEntry& entry = stringCodeLookup_[index]; // TODO: bounds check!
	if (entry.counterOfs == 0)
	{
		const uint8_t* str = stringTable_ + entry.stringOfs;
		StringStatistics::CounterOfs ofs = strings_.getCounter(str);
		if (ofs == 0)
		{
			flush();
			ofs = strings_.getCounter(str);
			// Second attempt must succeed
			assert(ofs);
		}
		entry.counterOfs = ofs;
	}
	strings_.counterAt(entry.counterOfs)->add(keys, values);
}

int AnalyzerContext::countStrings(protobuf::Message strings, int keys, int values)
{
	int numberOfStrings = 0;
	const uint8_t* p = strings.start;
	while (p < strings.end)
	{
		uint32_t strIndex = readVarint32(p);
		countString(strIndex, keys, values);
		numberOfStrings++;
	}
	return numberOfStrings;
}


void AnalyzerContext::stringTable(protobuf::Message strings)  // CRTP override
{
	stringTable_ = strings.start;
	const uint8_t* p = strings.start;
	while (p < strings.end)
	{
		uint32_t marker = readVarint32(p);
		if (marker != STRINGTABLE_ENTRY)
		{
			throw OsmPbfException("Bad string table. Unexpected field: %d", marker);
		}
		uint32_t ofs = p - strings.start;
		p += readVarint32(p);
		stringCodeLookup_.push_back({ ofs, 0 });
	}
	assert(p == strings.end);
}


void AnalyzerContext::endBlock()	// CRTP override
{
	stringCodeLookup_.clear();
}

void AnalyzerContext::afterTasks()
{
	LOG("Context %p: flushing remaining strings...", this);
	flush();
}

void Analyzer::processTask(AnalyzerOutputTask& task)
{
	const uint8_t* arena = task.strings();
	uint32_t size = *reinterpret_cast<const uint32_t*>(arena);
	const uint8_t* arenaEnd = arena + size;
	const uint8_t* p = arena + sizeof(uint32_t);
	while (p < arenaEnd)
	{
		const StringStatistics::Counter* counter =
			reinterpret_cast<const StringStatistics::Counter*>(p);
		uint32_t stringSize = StringStatistics::stringSize(counter->bytes);
		for(;;)
		{
			if (counter->total() < minStringCount_) break;
			StringStatistics::CounterOfs ofs;
			ofs = strings_.getCounter(counter->bytes, stringSize, counter->hash);
			if (ofs)
			{
				strings_.counterAt(ofs)->add(counter->keys, counter->values);
				break;
			}
			LOG("==== Global string arena full, culling strings < %d...", minStringCount_);
			strings_.removeStrings(minStringCount_++);
		}
		p += StringStatistics::Counter::grossSize(stringSize);
	}
	progress_.progress(task.blockBytesProcessed());
}


void Analyzer::analyze(const char* fileName)
{
	read(fileName, &progress_);
}
