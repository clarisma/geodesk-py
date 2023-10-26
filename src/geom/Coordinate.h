// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>

class Coordinate
{
public:
	constexpr Coordinate(int32_t x_, int32_t y_) : x(x_), y(y_) {}
	Coordinate(double x_, double y_) :
		x(static_cast<int32_t>(std::round(x_))),
		y(static_cast<int32_t>(std::round(y_))) {}

	bool operator== (const Coordinate& other) const
	{
		// return x==other.x && y==other.y;			// less efficient
		return std::memcmp(this, &other, sizeof(Coordinate)) == 0;
	}

	operator int64_t() const
	{
		return (static_cast<int64_t>(y) << 32) | static_cast<uint32_t>(x);
	}

	bool isNull() const { return static_cast<int64_t>(*this) == 0; };
	// bool isNull() const { return (x | y) == 0; };

	int32_t	x;
	int32_t	y;
};

// Make Coordinate hashable

namespace std 
{
	template<>
	struct hash<Coordinate> 
	{
		size_t operator()(const Coordinate& c) const 
		{
			return hash<int>()(c.x) ^ (hash<int>()(c.y) << 1);
			// Shift y's hash to ensure better distribution
		}
	};
}


static_assert(sizeof(Coordinate) == 8, "Compiler is padding Coordinate structure");