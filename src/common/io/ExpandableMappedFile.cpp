// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "ExpandableMappedFile.h"
#include <cassert>
#include <common/util/Bits.h>

ExpandableMappedFile::ExpandableMappedFile() :
	mainMapping_(nullptr),
	mainMappingSize_(0)
{
	memset(extendedMappings_, 0, sizeof(extendedMappings_));
}


void ExpandableMappedFile::open(const char* filename, int /* OpenMode */ mode)
{
	File::open(filename, mode | OpenMode::SPARSE);
		// must always be a sparse file
	uint64_t fileSize = size();
	if (mode & OpenMode::WRITE)
	{
		mainMappingSize_ = ((fileSize | 1) + SEGMENT_LENGTH - 1) & ~SEGMENT_LENGTH_MASK;
		setSize(mainMappingSize_);
		// TODO: only needed on Linux, Windows expands the file automatically
		// to match the mapping extent (No! It's in reverse)
	}
	else
	{
		mainMappingSize_ = fileSize;
	}
	mainMapping_ = reinterpret_cast<uint8_t*>(map(0, mainMappingSize_, 
		mode & (MappingMode::READ | MappingMode::WRITE)));
}


uint8_t* ExpandableMappedFile::translate(uint64_t ofs)
{
	if (ofs < mainMappingSize_) return mainMapping_ + ofs;
	ofs -= mainMappingSize_;
	uint64_t ofsBits = (ofs >> (SEGMENT_LENGTH_SHIFT - 1)) | 1;
		// we set 0-bit to 1 so we can use the slightly more efficient bit count
		// (that doesn't work if a value is zero)
	int slot = 63 - Bits::countLeadingZerosInNonZero64(ofsBits);
	assert(slot < EXTENDED_MAPPINGS_SLOT_COUNT);
	uint8_t* mapping = const_cast<uint8_t*>(extendedMappings_[slot]);
	if (!mapping) mapping = createExtendedMapping(slot);
	return mapping + ofs - (SEGMENT_LENGTH << slot) + SEGMENT_LENGTH;
}


uint8_t* ExpandableMappedFile::createExtendedMapping(int slot)
{
	// We use double-checked locking to safely create extended mappings
	// from multiple threads

	std::unique_lock<std::mutex> lock(extendedMappingsMutex_);
	uint8_t* mapping = const_cast<uint8_t*>(extendedMappings_[slot]);
	if (!mapping)
	{
		uint64_t size = SEGMENT_LENGTH << slot;
		uint64_t ofs = (size - SEGMENT_LENGTH) + mainMappingSize_;
		// TODO: may need to extend the file explicitly!
		setSize(ofs + size);
		mapping = reinterpret_cast<uint8_t*>(map(ofs, size, MappingMode::READ | MappingMode::WRITE));
		extendedMappings_[slot] = mapping;
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
			uint8_t* mapping = const_cast<uint8_t*>(extendedMappings_[slot]);
			unmap(mapping, mappingSize);
			extendedMappings_[slot] = nullptr;
		}
	}
}


uint8_t* ExpandableMappedFile::mapping(int n) 
{
	assert(n >= 0 && n <= EXTENDED_MAPPINGS_SLOT_COUNT);
	if(n == 0) return mainMapping_; 
	uint8_t* mapping = const_cast<uint8_t*>(extendedMappings_[n-1]);
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
	uint64_t ofsBits = ((ofs - mainMappingSize_ ) >> (SEGMENT_LENGTH_SHIFT - 1)) | 1;
	// we set 0-bit to 1 so we can use the slightly more efficient bit count
	// (that doesn't work if a value is zero)
	int slot = Bits::countLeadingZerosInNonZero64(ofsBits) - 1;
	assert(slot < EXTENDED_MAPPINGS_SLOT_COUNT);
	return slot + 1;
}


