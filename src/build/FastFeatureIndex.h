// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <common/store/IndexFile.h>

/**
 * A utility class that allows multiple threads to efficiently write 
 * the pile numbers of features into an index. Since indexes are compressed
 * arrays, write operations cannot be guaranteed to be atomic; absent of
 * synchronization, this would mean that two threads writing into nearby 
 * slots risk clobbering each other's work and thereby corrupting the 
 * index. In Java, we worked around this issue by having the workers threads
 * write the features' IDs and piles into a temporary array, which they then 
 * pass to the output thread, which then writes them into the index. 
 * 
 * However, in C++ we're able to allow the worker threads to write entries
 * directly into the index, with only minimal synchronization overhead, by
 * taking advantage of the following assumptions:
 * 
 * 1. Individual 64-bit words can be written concurrently without synchronization,
 *    as long as they are aligned (Visibility is not a concern, even on archs
 *    with a weal memory model, as other threads will only read these entries
 *    after passing through a mutex, which guarantees full synchronization)
 * 
 * 2. Features are stored in the OSM-PBF file in ascending order of their ID.
 *    This means that each block contains a range of IDs that are guaranteed
 *    not to overlap with the ranges of any other blocks. This, in turn, means
 *    that only entries at the beginning and end of the block's ID range can
 *    potentially conflict with entries of another block. These entries must be
 *    stored using an atomic operation. However, entries that lie in the interior
 *    can be written normally without risking a conflict, resulting in a 
 *    significant increase in throughput.
 */
class FastFeatureIndex
{
public:
	FastFeatureIndex(uint64_t* index, int valueWidth, int64_t maxId);

	enum WriteState
	{
		NOT_STARTED = -1,
		AT_START = 0,
		BEYOND_START = 1
	};

	int get(int64_t id);
	void put(int64_t id, int pile);
	void endBatch();
	
private:
	static const int64_t SEGMENT_LENGTH_BYTES = 1024 * 1024 * 1024;		// 1 GB

	struct CellRef
	{
		uint64_t* p;
		int bitOffset;
	};

	CellRef access(int64_t id);
	void flush();
	void flushAtomic();

	uint64_t* index_;
	int64_t maxId_;
	uint64_t* currentCell_;
	uint64_t cellData_;
	int writeState_;
	int valueWidth_;
	int slotsPerSegment_;
};
