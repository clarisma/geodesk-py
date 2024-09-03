// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cassert>
#include <cstdint>

/**
 * A 32-bit pointer, limited to a 4GB address space.
 * Since we are working in a 64-bit environment, we have to resolve
 * this kind of pointer against a base pointer in order to dereference it.
 */

// TODO: Should offset be based on bytes or sizeof(T)?

template <typename T>
class Ptr32
{
public:
	Ptr32(uint32_t ofs) : ofs_(ofs) {}
	Ptr32(T* p, T* base) : ofs_(static_cast<uint32_t>(p - base))
	{
		assert(p >= base);
		assert(ofs_ == p - base_)
	}

	T* operator(T* base) { return base + ofs_; }

private:
	uint32_t ofs_;
};
