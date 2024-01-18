#include "IndexFile.h"
#include <common/util/pointer.h>

uint32_t IndexFile::get(uint64_t key)
{
	assert(isOpen());
	assert(bits_);		// Index width must have been set
	// The block where the value is located
	uint64_t block = key / slotsPerBlock_;
	// The slot within the block
	uint32_t slot = static_cast<uint32_t>(key % slotsPerBlock_);
	// The byte within the block where the value starts
	uint32_t byteOffset = slot * bits_ / 8;
	// The bit within the first byte where the value starts
	uint32_t bitShift = slot * bits_ - byteOffset * 8;
	// Since we always read a full uint32_t even though the value takes up
	// fewer bits, we may run into a situation where we read bytes beyond 
	// the block, which may result in a segfault; to avoid this, we start 
	// reading at the offset that is <overrun> bytes earlier, and right-shift 
	// the value by 8 * <overrun> bits to compensate
	uint32_t overrun = std::max(
		static_cast<int32_t>(byteOffset) - 
		static_cast<int32_t>(BLOCK_SIZE - 4), 0);
	byteOffset -= overrun;
	bitShift += overrun * 8;
	pointer p(translate(block * BLOCK_SIZE + byteOffset));
	return (p.getUnalignedUnsignedInt() >> bitShift) & mask_;
}

void IndexFile::put(uint64_t key, uint32_t value)
{
	assert(isOpen());
	assert(bits_);		// Index width must have been set
	assert((value & mask_) == value);	// value must fit in range
	// The block where the value is located
	uint64_t block = key / slotsPerBlock_;
	// The slot within the block
	uint32_t slot = static_cast<uint32_t>(key % slotsPerBlock_);
	// The byte within the block where the value starts
	uint32_t byteOffset = slot * bits_ / 8;
	// The bit within the first byte where the value starts
	uint32_t bitShift = slot * bits_ - byteOffset * 8;
	// See above -- start reading earlier if the memory access would 
	// otherwise exceed the block length (potential segfault!), and 
	// shift the value to compensate
	uint32_t overrun = std::max(
		static_cast<int32_t>(byteOffset) -
		static_cast<int32_t>(BLOCK_SIZE - 4), 0);
	byteOffset -= overrun;
	bitShift += overrun * 8;
	void* pRaw = translate(block * BLOCK_SIZE + byteOffset);
	pointer p(pRaw);
	uint32_t oldValue = p.getUnalignedUnsignedInt();
	// TOOD: make this safe for architectures that don't allow unaligned writes
	*reinterpret_cast<uint32_t*>(pRaw) = 
		(oldValue & ~(mask_ << bitShift)) | (value << bitShift);
}
