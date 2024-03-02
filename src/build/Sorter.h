// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <vector>
#include <common/util/BufferWriter.h>
#include <common/util/ChunkedBuffer.h>
#include "geom/Coordinate.h"
#include "osm/OsmPbfReader.h"
#include "build/util/StringCatalog.h"
#include "PileWriter.h"

class GolBuilder;

class GroupEncoder
{
public:
	GroupEncoder(Arena& arena, int pile) :
		pile_(pile),
		prevId_(0),
		prevCoord_(0, 0),
		nextIndexed_(nullptr),
		nextSequential_(nullptr),
		buf_(arena, 16 * 1024)
	{
	}

private:
	GroupEncoder* nextIndexed_;
	GroupEncoder* nextSequential_;
	int pile_;
	int64_t prevId_;
	Coordinate prevCoord_;
	ChunkedBuffer buf_;
};

class Sorter;

enum SorterPhase
{
	NODES,
	WAYS,
	RELATIONS
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
	void way(int64_t id, protobuf::Message keys, protobuf::Message values, protobuf::Message nodes);
	void relation(int64_t id, protobuf::Message keys, protobuf::Message values,
		protobuf::Message roles, protobuf::Message memberIds, protobuf::Message memberTypes);
	void afterTasks();
	void harvestResults();

private:
	void encodeTags(protobuf::Message keys, protobuf::Message values);
	void encodeTags(protobuf::Message tags);
	void encodeString(uint32_t stringNumber, int type);
	void writeWay(uint32_t pile, uint64_t id);
	void addFeature(uint64_t id, uint32_t pile);
	void flush(int futurePhase);
	size_t batchSize(int phase) { return 1024 * 8192; }  // TODO

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
	// std::vector<FeatureIndexEntry> features_;
	int currentPhase_;

	std::vector<uint64_t> memberIds_;
	std::vector<uint32_t> tagsOrRoles_;
	uint64_t nodeCount_;
	uint64_t wayCount_;
	uint64_t wayNodeCount_;
	uint64_t relationCount_;
	uint64_t batchCount_;
};

class SorterOutputTask : public OsmPbfOutputTask
{
public:
	SorterOutputTask() {} // TODO: not needed, only to satisfy compiler
	SorterOutputTask(int currentPhase, int futurePhase,
		uint64_t bytesProcessed,
		// std::vector<FeatureIndexEntry>&& features, 
		PileSet&& piles) :
		currentPhase_(currentPhase),
		futurePhase_(futurePhase),
		bytesProcessed_(bytesProcessed),
		// features_(std::move(features)),
		piles_(std::move(piles))
	{
	}

	// SorterOutputTask

	// std::vector<FeatureIndexEntry> features_;
	PileSet piles_;
	uint64_t bytesProcessed_;
	int currentPhase_;
	int futurePhase_;
};

class Sorter : public OsmPbfReader<Sorter, SorterContext, SorterOutputTask>
{
public:
	Sorter(GolBuilder* builder);
	GolBuilder* builder() { return builder_; };
	void sort(const char* fileName);
	void processTask(SorterOutputTask& task);
	void addCounts(uint64_t nodeCount, uint64_t wayCount,
		uint64_t wayNodeCount, uint64_t relationCount)
	{
		nodeCount_ += nodeCount;
		wayCount_ += wayCount;
		wayNodeCount_ += wayNodeCount;
		relationCount_ += relationCount;
	}

private:
	ProgressReporter progress_;
	GolBuilder* builder_;
	uint64_t nodeCount_;
	uint64_t wayCount_;
	uint64_t wayNodeCount_;
	uint64_t relationCount_;
};
