// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "Analyzer.h"
#include "GolBuilder.h"
#include "build/util/StringCatalog.h"
#include <string>

// TODO: Need to flush remaining strings at end
// TODO: race condition

Analyzer::Analyzer(GolBuilder* builder) :
	OsmPbfReader(builder->threadCount()),
	builder_(builder),
	strings_(outputTableSize(), outputArenaSize()),
	minStringCount_(2)
{
}

AnalyzerContext::AnalyzerContext(Analyzer* analyzer) :
	OsmPbfContext<AnalyzerContext, Analyzer>(analyzer),
	nodeCounts_(new uint32_t[FastTileCalculator::GRID_CELL_COUNT + 1]),		
		// +1 to count any rejected nodes outside of range
	strings_(analyzer->workerTableSize(), analyzer->workerArenaSize())
{
	memset(nodeCounts_.get(), 0, (FastTileCalculator::GRID_CELL_COUNT + 1) * sizeof(uint32_t));
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
	uint32_t cell = reader()->tileCalculator()->calculateCell(lon100nd, lat100nd);
	nodeCounts_[cell]++;

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
	assert(index < stringCodeLookup_.size());  // TODO: exception?
	StringLookupEntry& entry = stringCodeLookup_[index]; 
	if (entry.counterOfs == 0)
	{
		const ShortVarString* str = reinterpret_cast<const ShortVarString *>(
			stringTable_ + entry.stringOfs);
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
		if (marker != OsmPbf::STRINGTABLE_ENTRY)
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

void AnalyzerContext::harvestResults()
{
	Analyzer* analyzer = reader();
	analyzer->mergeNodeCounts(nodeCounts_.release());
	analyzer->mergeStats(stats_);
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
		uint32_t stringSize = counter->string().totalSize();
		for(;;)
		{
			if (counter->totalCount() < minStringCount_) break;
			StringStatistics::CounterOfs ofs;
			ofs = strings_.getCounter(&counter->string(), counter->hash());
			if (ofs)
			{
				strings_.counterAt(ofs)->add(counter);
				break;
			}
			LOG("==== Global string arena full, culling strings < %d...", minStringCount_);
			strings_.removeStrings(minStringCount_);
			minStringCount_ <<= 1;
		}
		p += StringStatistics::Counter::grossSize(stringSize);
	}
	builder_->progress(task.blockBytesProcessed() * workPerByte_);
}


void Analyzer::mergeNodeCounts(uint32_t* counts)
{
	if (!totalNodeCounts_.get())
	{
		totalNodeCounts_.reset(counts);
	}
	else
	{
		uint32_t* pTotals = totalNodeCounts_.get();
		for (int i = 0; i < FastTileCalculator::GRID_CELL_COUNT + 1; i++)
		{
			pTotals[i] += counts[i];
		}
		delete[] counts;
	}
}


void Analyzer::mergeStats(const OsmStatistics& stats)
{
	totalStats_ += stats;
}


void Analyzer::addRequiredStrings()
{
	// TODO: need to init string count, ensure that required strings
	// cannot be evicted
	for (int i = 0; i < StringCatalog::CORE_STRING_COUNT; i++)
	{
		strings_.addRequiredCounter(std::string_view(StringCatalog::CORE_STRINGS[i]));
	}
	for (std::string_view str : builder_->settings().indexedKeyStrings())
	{
		strings_.addRequiredCounter(str);
	}
}

void Analyzer::startFile(uint64_t size)		// CRTP override
{
	workPerByte_ = builder_->phaseWork(GolBuilder::Phase::ANALYZE) / size;
	builder_->console().setTask("Analyzing...");
}

void Analyzer::analyze(const char* fileName)
{
	addRequiredStrings();
	read(fileName);
	// TODO: Only if verbose
	char buf[100];
	Format::unsafe(buf, "Analyzed %ld nodes and %ld",
		totalStats_.nodeCount,
		totalStats_.tagCount * 2 + totalStats_.memberCount);
	// progress_.end(buf);

	uint64_t totalStringCount = 0;
	uint64_t totalStringUsageCount = 0;
	StringStatistics::Iterator iter = strings_.iter();
	for (;;)
	{
		const StringStatistics::Counter* counter = iter.next();
		if (!counter) break;
		uint64_t subTotal = counter->trueTotalCount();
		if (subTotal >= 100)
		{
			std::string_view s = counter->stringView();
			totalStringCount++;
			totalStringUsageCount += subTotal;
		}
	}

	uint64_t literalsCount = totalStats_.tagCount * 2 + totalStats_.memberCount
		- totalStringUsageCount;

	Console::msg("  %12llu nodes", totalStats_.nodeCount);
	Console::msg("  %12llu ways", totalStats_.wayCount);
	Console::msg("  %12llu relations", totalStats_.relationCount);
	Console::msg("  %12llu members", totalStats_.memberCount);
	Console::msg("  %12llu tags", totalStats_.tagCount);
	Console::msg("  %12llu unique strings in string table", totalStringCount);
	Console::msg("  %12llu unique-string occurrences", totalStringUsageCount);
	Console::msg("  %12llu literal strings", literalsCount);

	Console::msg("Analysis complete.");
}


// TODO: Need to produce:
// - Global String Table
//   - hard-coded strings come first
//   - then indexed keys
//   - then the most common strings up to #127
//   - then all other keys up to KEY_MAX
//   - finally, remaining keys/values 
//   (make sure not to duplicate)
// - Lookup string -> encoded varint for key & value
// - Lookup of Proto-GOL String Code to GST Code, number or literal string

// - Get all strings & their key counts
// - Sort by key

