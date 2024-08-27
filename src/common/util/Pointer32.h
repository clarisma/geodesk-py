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
template <typename T>
class Pointer32
{
public:
	Pointer32(uint32_t ofs) : ofs_(ofs) {}
	static Pointer32<T> fromPointerAndBase(T* p, T* base)
	{
		assert(p >= base);
		ptrdiff_t ofs = p - base;
		assert(ofs < (1ULL << 32));
		return Pointer32<T>(static_cast<uint32_t>(ofs));
	}

	T* resolve(T* base) { return base + ofs_; }

private:
	uint32_t ofs_;
};
