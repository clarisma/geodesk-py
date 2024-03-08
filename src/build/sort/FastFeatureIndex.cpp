// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "FastFeatureIndex.h"
#include <atomic>


// TODO: Range checks on ID!

static_assert(sizeof(uint64_t) == sizeof(std::atomic<uint64_t>),
	"Extra state is maintained for std::atomic, the types are definitely "
	"not compatible on this platform.");

FastFeatureIndex::FastFeatureIndex(uint64_t* index, int valueWidth, int64_t maxId) :
	index_(index),
	valueWidth_(valueWidth),
	maxId_(maxId),
	currentCell_(nullptr),
	cellData_(0),
	writeState_(NOT_STARTED)
{
	slotsPerSegment_ = SEGMENT_LENGTH_BYTES * 8 / valueWidth;
}


FastFeatureIndex::CellRef FastFeatureIndex::access(int64_t id)
{
	lldiv_t d = lldiv(id, slotsPerSegment_);
	int64_t segmentNo = d.quot;
	int64_t slot = d.rem;
	uint64_t* pSegment = index_ + SEGMENT_LENGTH_BYTES / sizeof(uint64_t) * segmentNo;
	d = lldiv(slot * valueWidth_, 8 * sizeof(int64_t));
	CellRef ref;
	ref.p = pSegment + d.quot;
	ref.bitOffset = static_cast<int>(d.rem);
	return ref;
}


int FastFeatureIndex::get(int64_t id)
{
	if (id > maxId_) return 0;
		// Not an error; this means a relation/way refers to a missing feature
		// TODO: What about negative IDs?
		// --> should check in Sorter and reject the file

	CellRef ref = access(id);
	uint64_t mask = (1 << valueWidth_) - 1;
	uint64_t v = *ref.p >> ref.bitOffset;
	int overflow = ref.bitOffset + valueWidth_ - 64;
	if (overflow > 0)
	{
		v |= *(ref.p + 1) << (valueWidth_ - overflow);
	}
	return v & mask;
}


void FastFeatureIndex::flush()
{
	if (writeState_ > AT_START)
	{
		*currentCell_ = cellData_;
		// write directly, no risk of concurrent writes
	}
	else
	{
		if (writeState_ == AT_START)
		{
			flushAtomic();
			// The first cell affected by write operations in the current batch
			// must be written atomically, since it may contain one or more slots 
			// at the end of another batch
		}
		writeState_++;
	}
	cellData_ = 0;
}


void FastFeatureIndex::flushAtomic()
{
	std::atomic<uint64_t>* pAtomic = reinterpret_cast<std::atomic<uint64_t>*>(currentCell_);
	uint64_t oldValue = pAtomic->fetch_or(cellData_);
	// TODO: add checks to see if the slot to which we're writing is blank
	// If not, this means duplicate/out-of-order writes
}

void FastFeatureIndex::put(int64_t id, int pile)
{
	assert(id <= maxId_);
		// Unlike get(), writing an out-of-range ID si an error, because
		// we should be reading the same data as the Analyzer
		// TODO: What about negative IDs?
		// --> should check in Sorter and reject the file

	CellRef ref = access(id);
	assert(ref.p >= currentCell_);
	if (ref.p != currentCell_) flush();
	currentCell_ = ref.p;
	cellData_ |= static_cast<uint64_t>(pile) << ref.bitOffset;
	int overflow = ref.bitOffset + valueWidth_ - 64;
	if (overflow > 0)
	{
		flush();
		currentCell_++;
		cellData_ = static_cast<uint64_t>(pile) >> (valueWidth_ - overflow);
	}
}

void FastFeatureIndex::endBatch()
{
	// If there is a cell with a pending value, it must always be written
	// atomically, since it may be contain cells that belong to the start
	// of another block
	if (currentCell_) flushAtomic();
	currentCell_ = nullptr;
	cellData_ = 0;
	writeState_ = NOT_STARTED;
}

