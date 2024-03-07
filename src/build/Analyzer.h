// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include "osm/OsmPbfReader.h"
#include "FastTileCalculator.h"
#include "build/util/StringStatistics.h"
#include "OsmStatistics.h"

class Analyzer;
class GolBuilder;

class AnalyzerContext : public OsmPbfContext<AnalyzerContext, Analyzer>
{
public:
	AnalyzerContext(Analyzer* analyzer);

	// CRTP overrides
	void stringTable(protobuf::Message strings);
	void endBlock(); 
	void node(int64_t id, int32_t lon100nd, int32_t lat100nd, protobuf::Message& tags);
	void way(int64_t id, protobuf::Message keys, protobuf::Message values, protobuf::Message nodes);
	void relation(int64_t id, protobuf::Message keys, protobuf::Message values,
		protobuf::Message roles, protobuf::Message memberIds, protobuf::Message memberTypes);
	void afterTasks();
	void harvestResults();
	
private:
	void flush();
	void countString(uint32_t index, int keys, int values);
	int countStrings(protobuf::Message strings, int keys, int values);

	struct StringLookupEntry
	{
		uint32_t stringOfs;
		uint32_t counterOfs;
	};


	std::unique_ptr<uint32_t[]> nodeCounts_;

	/**
	 * Pointer to the string table of the current block.
	 * (This memory is not owned, it is managed by OsmPbfReader)
	 * This pointer is only valid for the currently processed block.
	 */
	const uint8_t* stringTable_;

	/**
	 * A table that translates an OSM string code to either a string in the
	 * current block's string table (If the string is encountered for the 
	 * first time in this block) or a StringStatistics::Counter.
	 * We use offsets instead of pointers to cut size in half.
	 * We use bit 0 to discriminate the two types:
	 *	Bit 0 = 0: string in string-table
	 *  Bit 0 = 1: StringStatistics::Counter
	 *  Bit 1-31: offset (can address 2 GB)
	 * This approach avoids having to look up the string's Counter more 
	 * than once per OSM block. We could obtain all Counters at the start
	 * of the block (when the OsmPbfReader reads the string table), but this
	 * complicates the flushing mechanism (We may run out of space in the local
	 * arena in the middle of the string table), so we build it lazily instead.
	 */
	std::vector<StringLookupEntry> stringCodeLookup_;
	StringStatistics strings_;
	OsmStatistics stats_;
};

class AnalyzerOutputTask : public OsmPbfOutputTask
{
public:
	AnalyzerOutputTask() {} // TODO: not needed, only to satisfy compiler
	AnalyzerOutputTask(const uint8_t* strings, uint64_t blockBytesProcessed) :
		strings_(strings),
		blockBytesProcessed_(blockBytesProcessed)
	{
	}

	// TODO: Needs to be move-constructable??

	const uint8_t* strings() const { return strings_.get(); }
	const uint64_t blockBytesProcessed() const { return blockBytesProcessed_; }

private:
	std::unique_ptr<const uint8_t[]> strings_;
	// size_t currentBatchStringCount_;
	uint64_t blockBytesProcessed_;
};

class Analyzer : public OsmPbfReader<Analyzer, AnalyzerContext, AnalyzerOutputTask>
{
public:
	Analyzer(GolBuilder* builder);

	uint32_t workerTableSize() { return 2 * 1024 * 1024; }
	uint32_t workerArenaSize() { return 16 * 1024 * 1024; }
	uint32_t outputTableSize() { return 4 * 1024 * 1024; }
	uint32_t outputArenaSize() { return 32 * 1024 * 1024; }

	void analyze(const char* fileName);
	void startFile(uint64_t size);		// CRTP override
	void processTask(AnalyzerOutputTask& task);
	const FastTileCalculator* tileCalculator() const { return &tileCalculator_; }
	
	/**
	 * Adds the given node counts to the total. This method takes ownership
	 * of the pointer.
	 */
	void mergeNodeCounts(uint32_t* counts);
	void mergeStats(const OsmStatistics& stats);
	const OsmStatistics& osmStats() const { return totalStats_; }
	const StringStatistics& strings() const { return strings_; }
	std::unique_ptr<const uint32_t[]> takeTotalNodeCounts()
	{
		return std::unique_ptr<const uint32_t[]>(totalNodeCounts_.release());
	}

private:
	void addRequiredStrings();

	GolBuilder* builder_;
	StringStatistics strings_;
	const FastTileCalculator tileCalculator_;
	int minStringCount_;
	std::unique_ptr<uint32_t[]> totalNodeCounts_;
	OsmStatistics totalStats_;
	double workPerByte_;
};
