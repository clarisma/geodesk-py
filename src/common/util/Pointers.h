// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cassert>
#include <cstdint>

namespace Pointers
{

inline int32_t nearDelta(const void* dest, const void* source)
{
	ptrdiff_t d = reinterpret_cast<const uint8_t*>(dest) - reinterpret_cast<const uint8_t*>(source);
	assert(d == static_cast<int32_t>(d));
	return static_cast<int32_t>(d);
}

inline uint32_t nearOffset(const void* dest, const void* source)
{
	assert(dest >= source);
	ptrdiff_t d = reinterpret_cast<const uint8_t*>(dest) - reinterpret_cast<const uint8_t*>(source);
	assert(d == static_cast<uint32_t>(d));
	return static_cast<uint32_t>(d);
}

}
