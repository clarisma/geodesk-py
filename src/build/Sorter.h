// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <vector>
#include <common/util/ChunkedBuffer.h>
#include "geom/Coordinate.h"
#include "osm/OsmPbfReader.h"
#include "ProtoStringTable.h"

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

class SorterContext : public OsmPbfContext<SorterContext, Sorter>
{
public:
	SorterContext(Sorter* sorter);
	~SorterContext();

	// CRTP overrides
	void stringTable(protobuf::Message strings);
	void node(int64_t id, int32_t lon100nd, int32_t lat100nd, protobuf::Message& tags);
	void way(int64_t id, protobuf::Message keys, protobuf::Message values, protobuf::Message nodes);
	void relation(int64_t id, protobuf::Message keys, protobuf::Message values,
		protobuf::Message roles, protobuf::Message memberIds, protobuf::Message memberTypes);

private:
	GroupEncoder* getEncoder(int pile, int startMarker);
	void encodeTags(protobuf::Message keys, protobuf::Message values);
	void encodePackedString(uint32_t blockStringCode, ProtoStringType keyOrValue);

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
	std::vector<ProtoStringEncoding> stringTranslationTable_;
	BufferWriter tempWriter_;
	std::vector<uint64_t> memberIds_;
	std::vector<uint32_t> tagsOrRoles_;
};

class SorterOutputTask : public OsmPbfOutputTask
{
};

class Sorter : public OsmPbfReader<Sorter, SorterContext, SorterOutputTask>
{
public:
	Sorter(int numberOfThreads);

private:
	ProgressReporter progress_;
};
