// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "Analyzer.h"
#include <string>
#include "geom/Tile.h"
#include "geom/Mercator.h"

// TODO

AnalyzerContext::AnalyzerContext(Analyzer* analyzer) :
	OsmPbfContext<AnalyzerContext, Analyzer>(analyzer),
	nodeCounts_(nullptr),
	strings_(analyzer->workerTableSize(), analyzer->workerHeapSize())
{
	nodeCounts_ = new uint32_t[4096 * 4096];
	memset(nodeCounts_, 0, 4096 * 4096 * sizeof(uint32_t));
}

AnalyzerContext::~AnalyzerContext()
{
	if (nodeCounts_) delete[] nodeCounts_;
}

void AnalyzerContext::flush()
{
	// protobuf::Message strings = strings_.takeStrings();
	// TODO: submit task
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

	const uint8_t* p = tags.p;
	while (p < tags.pEnd)
	{
		uint32_t key = readVarint32(p);
		if (key == 0) break;
		uint32_t value = readVarint32(p);
		// countString(key, 1);
		// countString(value, 1);
		stats_.tagCount++;
	}
	tags.p = p;
	stats_.nodeCount++;
	stats_.maxNodeId = id;	// assumes nodes are ordered by ID
}


void AnalyzerContext::way(int64_t id, protobuf::Message keys, protobuf::Message values, protobuf::Message nodes)
{
	countStrings(keys, 1);
	stats_.tagCount += countStrings(values, 1);
	stats_.wayCount++;
	stats_.maxWayId = id;  // assumes ways are ordered by ID
}

void AnalyzerContext::relation(int64_t id, protobuf::Message keys, protobuf::Message values, 
	protobuf::Message roles, protobuf::Message memberIds, protobuf::Message memberTypes)
{
	countStrings(keys, 1);
	stats_.tagCount += countStrings(values, 1);
	stats_.memberCount += countStrings(roles, 1);
	stats_.relationCount++;
	stats_.maxRelationId = id;  // assumes relations are ordered by ID
}

void AnalyzerContext::countString(uint32_t index, int count)
{
	if (!strings_.addString(string(index), count))
	{
		flush();
		assert(strings_.addString(string(index), count));
	}
}

int AnalyzerContext::countStrings(protobuf::Message strings, int count)
{
	int numberOfStrings = 0;
	const uint8_t* p = strings.p;
	while (p < strings.pEnd)
	{
		uint32_t strIndex = readVarint32(p);
		// countString(strIndex, count);
		numberOfStrings++;
	}
	return numberOfStrings;
}
