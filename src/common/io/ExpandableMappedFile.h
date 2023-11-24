#pragma once
#include "Mappedfile.h"

class ExpandableMappedFile : private MappedFile
{
public:
	ExpandableMappedFile();

	void open(const char* filename, int /* OpenMode */ mode);
	uint8_t* translate(uint64_t ofs);

protected:
	static const uint64_t SEGMENT_LENGTH = 1024 * 1024 * 1024;		// 1 GB
	static const int SEGMENT_LENGTH_SHIFT = 30;
	static const int EXTENDED_MAPPINGS_SLOT_COUNT = 16;

	uint8_t* mainMapping_;
	size_t mainMappingSize_;
	uint8_t* extendedMappings_[EXTENDED_MAPPINGS_SLOT_COUNT];
};
