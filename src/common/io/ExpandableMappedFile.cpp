// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "ExpandableMappedFile.h"
#include <cassert>
#include <common/cli/Console.h>
#include <common/util/Bits.h>

ExpandableMappedFile::ExpandableMappedFile() :
	mainMapping_(nullptr),
	mainMappingSize_(0)
{
	for (int i = 0; i < EXTENDED_MAPPINGS_SLOT_COUNT; i++)
	{
		extendedMappings_[i].store(nullptr);
	}
}


void ExpandableMappedFile::open(const char* filename, int /* OpenMode */ mode)
{
	File::open(filename, mode | OpenMode::SPARSE);
		// must always be a sparse file
	uint64_t fileSize = size();
	if (mode & OpenMode::WRITE)
	{
        uint64_t segmentLen = SEGMENT_LENGTH;
            // This gets around the odr issue of using SEGMENT_LENGTH in call
            // to std::max (which expects a reference)
		mainMappingSize_ = (std::max(fileSize, segmentLen) + SEGMENT_LENGTH - 1) & ~SEGMENT_LENGTH_MASK;
		// round up to closest multiple of 1 GB (SEGMENT_LENGTH), with a 1 GB minimum
		#ifndef _WIN32
		setSize(mainMappingSize_);
		// Only needed on Linux, Windows expands the file automatically
		#endif
	}
	else
	{
		mainMappingSize_ = fileSize;
	}
	mainMapping_ = reinterpret_cast<uint8_t*>(map(0, mainMappingSize_, 
		mode & (MappingMode::READ | MappingMode::WRITE)));
	Console::msg("Created main mapping at %p (size %llu)", mainMapping_, mainMappingSize_);
}


uint8_t* ExpandableMappedFile::translate(uint64_t ofs)
{
	assert(ofs < MAX_FILE_SIZE);
	if (ofs < mainMappingSize_) return mainMapping_ + ofs;
	//printf("Getting extended mapping for offset %llu:\n", ofs);
	ofs -= mainMappingSize_;
	//printf("  Adjusted ofs = %llu\n", ofs);
	uint64_t ofsBits = (ofs + SEGMENT_LENGTH ) >> SEGMENT_LENGTH_SHIFT;
	int slot = 63 - Bits::countLeadingZerosInNonZero64(ofsBits);
	assert(slot >= 0);
	assert(slot < EXTENDED_MAPPINGS_SLOT_COUNT);
	//printf("  Slot = %d\n", slot);
	// Load with acquire ordering to ensure subsequent reads / writes are not reordered before this
	uint8_t* mapping = extendedMappings_[slot].load(std::memory_order_acquire);
	if (!mapping) mapping = createExtendedMapping(slot);
	assert(ofs >= (SEGMENT_LENGTH << slot) - SEGMENT_LENGTH);
	uint64_t offsetIntoSegment = ofs - (SEGMENT_LENGTH << slot) + SEGMENT_LENGTH;
	
	if (offsetIntoSegment >= mappingSize(slot + 1))
	{
		Console::msg("Offset %llu overruns size of slot %d (%llu)",
			offsetIntoSegment, slot, mappingSize(slot + 1));
	}
	
	// assert(offsetIntoSegment < mappingSize(slot + 1));
	return mapping + ofs - (SEGMENT_LENGTH << slot) + SEGMENT_LENGTH;
}


uint8_t* ExpandableMappedFile::createExtendedMapping(int slot)
{
	// We use double-checked locking to safely create extended mappings
	// from multiple threads

	std::unique_lock<std::mutex> lock(extendedMappingsMutex_);
	// Re-check with relaxed ordering since we're already protected by the mutex
	uint8_t* mapping = extendedMappings_[slot].load(std::memory_order_relaxed);
	if (!mapping)
	{
		uint64_t size = SEGMENT_LENGTH << slot;
		uint64_t ofs = (size - SEGMENT_LENGTH) + mainMappingSize_;
		
		// Windows automatically grows a file to accommodate mappings
		// On Linux, we need to extend the file explicitly
		// printf("Slot %d: Need to map %llu at %llu\n", slot, size, ofs);
		// printf("Trying to resize to %llu\n", ofs + size);
		#ifndef _WIN32
		setSize(ofs + size);
		// printf("  Resized.\n");
		#endif
		mapping = reinterpret_cast<uint8_t*>(map(ofs, size, MappingMode::READ | MappingMode::WRITE));
		extendedMappings_[slot].store(mapping, std::memory_order_release);

		Console::msg("Created extended mapping #%d at %llu (size %llu) -- p = %p",
			slot, ofs, size, mapping);
	}
	return mapping;
}


void ExpandableMappedFile::unmapSegments()
{
	std::unique_lock<std::mutex> lock(extendedMappingsMutex_);
	if (mainMapping_)
	{
		unmap(mainMapping_, mainMappingSize_);
		mainMapping_ = nullptr;
	}

	uint64_t mappingSize = SEGMENT_LENGTH;
	for (int slot = 0; slot < EXTENDED_MAPPINGS_SLOT_COUNT; slot++)
	{
		if (extendedMappings_[slot])
		{
			uint8_t* mapping = extendedMappings_[slot].load();
			unmap(mapping, mappingSize);
			extendedMappings_[slot].store(nullptr);
		}
	}
}

uint8_t* ExpandableMappedFile::mapping(int n) 
{
	assert(n >= 0 && n <= EXTENDED_MAPPINGS_SLOT_COUNT);
	if(n == 0) return mainMapping_; 
	uint8_t* mapping = extendedMappings_[n-1].load(std::memory_order_acquire);
	if (!mapping) mapping = createExtendedMapping(n-1);
	return mapping;
}

size_t ExpandableMappedFile::mappingSize(int n) const
{
	return n == 0 ? mainMappingSize_ : (SEGMENT_LENGTH << (n - 1));
}

int ExpandableMappedFile::mappingNumber(uint64_t ofs) const
{
	if (ofs < mainMappingSize_) return 0;
	uint64_t ofsBits = (ofs - mainMappingSize_ + SEGMENT_LENGTH) >> SEGMENT_LENGTH_SHIFT;
	int slot = 63 - Bits::countLeadingZerosInNonZero64(ofsBits);
	assert(slot < EXTENDED_MAPPINGS_SLOT_COUNT);
	return slot + 1;
}


