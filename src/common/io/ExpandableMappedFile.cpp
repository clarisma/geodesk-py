#include "ExpandableMappedFile.h"

ExpandableMappedFile::ExpandableMappedFile() :
	mainMapping_(nullptr),
	mainMappingSize_(0)
{
	memset(extendedMappings_, 0, sizeof(extendedMappings_));
}


void ExpandableMappedFile::open(const char* filename, int /* OpenMode */ mode)
{

}


uint8_t* ExpandableMappedFile::translate(uint64_t ofs)
{
	if (ofs < mainMappingSize_) return mainMapping_ + ofs;
	uint64_t ofsBits = ((ofs - mainMappingSize_) >> (SEGMENT_LENGTH_SHIFT - 1)) | 1;
		// we set 0-bit to 1 so we can use the slightly more efficient bit count
		// (that doesn't work if a value is zero)
	int slot = countTrailingZerosInNonZero
}
