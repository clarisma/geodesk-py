// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "MappedIndex.h"

uint64_t MappedIndex::calculateMappingSize()
{
	int64_t totalSlots = maxId_ + 1;
	int64_t slotsPerSegment = SEGMENT_LENGTH_BYTES * 8 / valueWidth_;
	int64_t fullSegments = totalSlots / slotsPerSegment;
	int64_t partialSegmentSlots = totalSlots % slotsPerSegment;
	return SEGMENT_LENGTH_BYTES * fullSegments +
		((partialSegmentSlots * valueWidth_ / 8 + 4096 - 1) & ~(4096 - 1));
	// TODO: check this, will cause segfault if calculation is wrong!
	// TODO: Ensure SEGMENT_LENGTH_BYTES is a 64-bit type
}

void MappedIndex::create(const char* fileName, int64_t maxId, int valueWidth)
{
	maxId_ = maxId;
	valueWidth_ = valueWidth;

	MappedFile file;
	file.open(fileName,
		File::OpenMode::READ | File::OpenMode::WRITE |
		File::OpenMode::REPLACE_EXISTING | File::OpenMode::SPARSE);
	uint64_t totalBytes = calculateMappingSize();
	file.setSize(totalBytes);
	index_ = reinterpret_cast<uint64_t*>(file.map(0, totalBytes,
		MappedFile::MappingMode::READ | MappedFile::MappingMode::WRITE));
}


void MappedIndex::release()
{
	if (index_) MappedFile::unmap(index_, calculateMappingSize());
}
	