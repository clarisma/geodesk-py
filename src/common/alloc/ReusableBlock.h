// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cassert>
#include <cstdint>
#include <memory>
#include <common/cli/Console.h>
#include <common/util/Bytes.h>

class ReusableBlock
{
public:
	ReusableBlock(uint32_t increments = 1024 * 1024, int maxWastefulCycles = 3) :
		capacity_(0),
		size_(0),
		increments_(increments),
		maxWastefulCycles_(static_cast<int16_t>(maxWastefulCycles))
	{
		assert(Bytes::isPowerOf2(increments));
		assert(maxWastefulCycles >= 0 && maxWastefulCycles < (1 << 16) - 1);
		wastefulCyclesRemaining_ = maxWastefulCycles_;
	}

	ReusableBlock(const ReusableBlock& other) = delete;

	ReusableBlock(ReusableBlock&& other) noexcept :
		data_(std::move(other.data_)),
		capacity_(other.capacity_),
		size_(other.size_),
		increments_(other.increments_),
		maxWastefulCycles_(other.maxWastefulCycles_),
		wastefulCyclesRemaining_(other.wastefulCyclesRemaining_)
	{
		other.size_ = 0;
		other.capacity_ = 0;
		other.wastefulCyclesRemaining_ = other.maxWastefulCycles_;
	}

	ReusableBlock& operator=(const ReusableBlock& other) = delete;

	ReusableBlock& operator=(ReusableBlock&& other) noexcept
	{
		if (this != &other)
		{
			data_ = std::move(other.data_);
			capacity_ = other.capacity_;
			size_ = other.size_;
			increments_ = other.increments_;
			maxWastefulCycles_ = other.maxWastefulCycles_;
			wastefulCyclesRemaining_ = other.wastefulCyclesRemaining_;

			other.size_ = 0;
			other.capacity_ = 0;
			other.wastefulCyclesRemaining_ = other.maxWastefulCycles_;
		}
		return *this;
	}

	uint8_t* data() const noexcept { return data_.get(); };
	size_t size() const noexcept { return size_; }
	size_t capacity() const noexcept { return capacity_; }

	void resize(size_t newSize) 
	{ 
		size_ = newSize;
		wastefulCyclesRemaining_ -= (capacity_ - newSize >= increments_) ? 1 : 0;
		// Console::debug("Resizing buffer to %llu (%d wasteful cycles remaining)", 
		//	newSize, wastefulCyclesRemaining_);

		if (newSize > capacity_ || wastefulCyclesRemaining_ == 0)
		{
			size_t newCapacity = (newSize + increments_ - 1) & 
				~(static_cast<size_t>(increments_) - 1);
			assert(newCapacity >= newSize);
			// assert(_CrtCheckMemory());
			// Console::debug("  Allocating %llu", newCapacity);
			uint8_t* d = new uint8_t[newCapacity];
			data_.reset(d);
			capacity_ = newCapacity;
			wastefulCyclesRemaining_ = maxWastefulCycles_;
			//Console::debug("Resized buffer to %llu, REALLOCATED %lld (%d retries)",
			//	newSize, capacity_, wastefulCyclesRemaining_);
			return;
		}
		//Console::debug("Resized buffer to %llu, keeping %lld (%d retries)",
		//	newSize, capacity_, wastefulCyclesRemaining_);
	}

private:
	std::unique_ptr<uint8_t[]> data_;
	size_t capacity_ = 0;
	size_t size_ = 0;
	uint32_t increments_;
	int16_t maxWastefulCycles_;
	int16_t wastefulCyclesRemaining_;
};

