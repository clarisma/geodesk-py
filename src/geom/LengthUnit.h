// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cassert>
#include <string_view>

class LengthUnit
{
public:
	static const int METERS = 0;
	static const int KILOMETERS = 1;
	static const int FEET = 2;
	static const int YARDS = 3;
	static const int MILES = 4;

	static double fromMeters(double meters, int unit)
	{
		assert(unit >= 0 && unit <= 4);
		return meters * METERS_TO_UNIT[unit];
	}

	static double toMeters(double units, int unit)
	{
		assert(unit >= 0 && unit <= 4);
		return units * UNITS_TO_METERS[unit];
	}

	/**
	 * Returns the length unit based on the given string,
	 * or -1 if it does not represent a valid unit type.
	 */
	static int unitFromString(std::string_view unit);

	static const double METERS_TO_UNIT[];
	static const double UNITS_TO_METERS[];

	static const char* VALID_UNITS;
};

