// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include "Bits.h"

template<typename T>
class BitIterator
{
public:
	BitIterator(T bits)
	{
		bits_ = bits;
		pos_ = 0;
	}

	/**
	 * Returns the number of the next non-zero bit in the bitfield.
	 * If none remain, returns -1
	 */
	int next()
	{
		if (bits_ == 0) return -1;
		int n = Bits::countTrailingZerosInNonZero(bits_);
		int res = pos_ + n;
		bits_ >>= n + 1;
		pos_ += n + 1;
		return res;
	}
	
private:
	T bits_;
	int pos_;
};

