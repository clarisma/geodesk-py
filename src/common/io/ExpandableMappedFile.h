// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include "MappedFile.h"
#include <mutex>

/**
 * A MappedFile that grows on demand (if opened in writable mode).
 * The main mapping covers the entire current file (rounded up to nearest 1 GB
 * if opened in writable mode). If regions are accessed that lie beyond the 
 * current size, the underlyong file is grown and additional maps are created. 
 * Growth is progressive: First, an additional 1 GB, then 2 GB, then 4 GB, etc.
 * up to a total of 64 TB. 
 * 
 * Restrictions:
 * - Underlying file is expected to be a sparse file
 * - Data cannot be accessed across 1-GB boundaries
 * 
 */
class ExpandableMappedFile : private MappedFile
{
public:
	ExpandableMappedFile();

	void open(const char* filename, int /* OpenMode */ mode);

	/**
	 * Obtains a pointer to the data which begins at the given 
	 * offset into the file. If the address lies beyond the
	 * current file size, the file is grown in installments
	 * of at least 1 GB.
	 * 
	 * To access data beyond the current file size, the file 
	 * must be opened with OpenMode::WRITE, or the behavior
	 * of this call will be undefined.
	 * 
	 * Any address within a 1-GB segment can be safely accessed; 
	 * however, reading across 1-GB boundaries is not allowed 
	 * (The next byte across a boundary may be in a different 
	 * mapping, and hence at a different virtual memory address).
	 */
	uint8_t* translate(uint64_t ofs);

protected:
	static const uint64_t SEGMENT_LENGTH = 1024 * 1024 * 1024;		// 1 GB
	static const int SEGMENT_LENGTH_SHIFT = 30;
	static const uint64_t SEGMENT_LENGTH_MASK = 0x3fff'ffff;
	static const int EXTENDED_MAPPINGS_SLOT_COUNT = 16;

private:
	uint8_t* createExtendedMapping(int slot);

	uint8_t* mainMapping_;
	size_t mainMappingSize_;
	uint8_t* extendedMappings_[EXTENDED_MAPPINGS_SLOT_COUNT];
	std::mutex extendedMappingsMutex_;
};
