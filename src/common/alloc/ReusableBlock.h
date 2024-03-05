// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cassert>
#include <cstdint>
#include <memory>
#include <common/util/Bytes.h>

template <uint32_t Increments, int MaxCycles>
class ReusableBlock<Increments, MaxCycles>
{
public:
	static_assert(Bytes::isPowerOf2(Increments), "Increments must be a power of 2");

	uint8_t* get(uint32_t requiredCapacity) 
	{ 
		assert(requiredCapacity != 0);
		if (requiredCapacity > capacity_ || !allowOverage_)
		{
			uint8_t* d = new uint_8[(requiredCapacity + Increments - 1) & ~Increments];
			data_.reset(d);
			allowOverage_ = MaxCycles;
			return d;
		}
		allowOverage_ -= (capacity_ - requiredCapacity >= Increments) ? 1 : 0;
		return data_.get();
	}

private:
	std::unique_ptr<uint8_t> data_;
	uint32_t capacity_ = 0;
	int allowOverage_ = MaxCycles;
};

