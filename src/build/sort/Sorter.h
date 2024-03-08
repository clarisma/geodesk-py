// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <vector>
#include <common/util/BufferWriter.h>
#include <common/util/BufferWriter.h>
#include <common/thread/Phaser.h>
#include "geom/Coordinate.h"
#include "osm/OsmPbfReader.h"
#include "build/util/StringCatalog.h"
#include "build/util/PileWriter.h"
#include "FastFeatureIndex.h"

class GolBuilder;

class Sorter;


struct SorterStatistics
{
	SorterStatistics()
	{
		memset(this, 0, sizeof(SorterStatistics));
	}

	SorterStatistics& operator+=(const SorterStatistics& other)
	{
		nodeCount += other.nodeCount;
		wayCount += other.wayCount;
		relationCount += other.relationCount;
		multitileWayCount += other.multitileWayCount;
		ghostWayCount += other.ghostWayCount;
		wayNodeCount += other.wayNodeCount;
		return *this;
	}

	int64_t nodeCount;
	int64_t wayCount;
	int64_t multitileWayCount;
	int64_t ghostWayCount;
	int64_t wayNodeCount;
	int64_t relationCount;
};

/*
class FeatureIndexEntry
{
public:
	FeatureIndexEntry(uint64_t id, uint32_t pile) :
		id_(id), pile_(pile) {}

	uint64_t id() const noexcept { return id_; }
	uint64_t pile() const noexcept { return pile_; }

private:
	uint64_t id_;
	uint32_t pile_;
};
*/

class SorterContext : public OsmPbfContext<SorterContext, Sorter>
{
public:
	SorterContext(Sorter* sorter);
	~SorterContext();

	// TODO: This constructor only exists to satify the requirements
	// of std:vector (used in TaskEngine)
	// It must never be called!
	SorterContext(const SorterContext& other) :
		OsmPbfContext(nullptr),
		tempBuffer_(4096),
		tempWriter_(&tempBuffer_),
		pileWriter_(0)
	{
		assert(false);
	}

	// CRTP overrides
	void stringTable(protobuf::Message strings);
	void node(int64_t id, int32_t lon100nd, int32_t lat100nd, protobuf::Message& tags);
	void beginWayGroup();
	void way(int64_t id, protobuf::Message keys, protobuf::Message values, protobuf::Message nodes);
	void beginRelationGroup();
	void relation(int64_t id, protobuf::Message keys, protobuf::Message values,
		protobuf::Message roles, protobuf::Message memberIds, protobuf::Message memberTypes);
	void endBlock();
	void afterTasks();
	void harvestResults();

private:
	void encodeTags(protobuf::Message keys, protobuf::Message values);
	void encodeTags(protobuf::Message tags);
	void encodeString(uint32_t stringNumber, int type);
	// void writeWay(uint32_t pile, uint64_t id);
	void addFeature(int64_t id, int pile);
	void advancePhase(int futurePhase);
	void flushPiles();
	void flushIndex();
	size_t batchSize(int phase) 
	{ 
		return phase == 0 ? (1024 * 1024) : (32 * 1024); 
	}  

	GolBuilder* builder_;
	/**
	 * Pointer to the start of the string table of the current OSM block.
	 */
	const uint8_t* osmStrings_;

	/**
	 * This vector helps turn the string references in the current OSM block
	 * into proto-string encodings. For each string in the current OSM block,
	 * it holds an entry that turns a key or value into a varint (the code in
	 * the Proto-String Table) or into a literal string (an offset into the
	 * OSM block's string table)
	 */
	std::vector<ProtoStringCode> stringTranslationTable_;
	DynamicBuffer tempBuffer_;	// keep this order
	BufferWriter tempWriter_;
	PileWriter pileWriter_;
	FastFeatureIndex indexes_[3];
	int currentPhase_;
	int pileCount_;

	std::vector<uint64_t> memberIds_;
	std::vector<uint32_t> tagsOrRoles_;
	SorterStatistics stats_;
	uint64_t batchCount_;
};

class SorterOutputTask : public OsmPbfOutputTask
{
public:
	SorterOutputTask() {} // TODO: not needed, only to satisfy compiler
	SorterOutputTask(uint64_t bytesProcessed, PileSet&& piles) :
		bytesProcessed_(bytesProcessed),
		piles_(std::move(piles))
	{
	}

	PileSet piles_;
	uint64_t bytesProcessed_;
};

class Sorter : public OsmPbfReader<Sorter, SorterContext, SorterOutputTask>
{
public:
	enum Phase { NODES, WAYS, RELATIONS, SUPER_RELATIONS };
	using SorterPhaser = Phaser<3>;

	Sorter(GolBuilder* builder);
	GolBuilder* builder() { return builder_; };
	// SorterPhaser& phaser() { return phaser_; }
	void sort(const char* fileName);
	void startFile(uint64_t size);		// CRTP override
	void processTask(SorterOutputTask& task);
	void advancePhase(int currentPhase, int newPhase);
	void addCounts(const SorterStatistics& stats)
	{
		stats_ += stats;
	}

private:
	GolBuilder* builder_;
	std::mutex phaseMutex_;
	std::condition_variable phaseStarted_;
	SorterStatistics stats_; 
	double workPerByte_;
	int phaseCountdowns_[3];
};
