// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include "osm/OsmPbfReader.h"
#include "StringStatistics.h"
#include "OsmStatistics.h"

class Analyzer;

class AnalyzerContext : public OsmPbfContext<AnalyzerContext, Analyzer>
{
public:
	AnalyzerContext(Analyzer* analyzer);
	~AnalyzerContext();

	void node(int64_t id, int32_t lon100nd, int32_t lat100nd, protobuf::Message& tags);
	void way(int64_t id, protobuf::Message keys, protobuf::Message values, protobuf::Message nodes);
	void relation(int64_t id, protobuf::Message keys, protobuf::Message values,
		protobuf::Message roles, protobuf::Message memberIds, protobuf::Message memberTypes);

private:
	void flush();
	void countString(uint32_t index, int count);
	int countStrings(protobuf::Message strings, int count);

	uint32_t* nodeCounts_;
	StringStatistics strings_;
	OsmStatistics stats_;
};

class AnalyzerOutputTask : public OsmPbfOutputTask
{
};

class Analyzer : public OsmPbfReader<Analyzer, AnalyzerContext, OsmPbfOutputTask>
{
public:
	Analyzer(int numberOfThreads) : OsmPbfReader(numberOfThreads) {}

	uint32_t workerTableSize() { return 1024 * 1024; }
	uint32_t workerHeapSize() { return 32 * 1024 * 1024; }
};
