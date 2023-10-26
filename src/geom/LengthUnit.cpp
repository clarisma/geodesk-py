// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "LengthUnit.h"
#include <cstring>

#include "LengthUnit_attr.cxx"


int LengthUnit::unitFromString(std::string_view unit)
{
	Unit* attr = LengthUnit_AttrHash::lookup(unit.data(), unit.length());
	if (attr) return attr->index;
	return -1;
}

const double LengthUnit::METERS_TO_UNIT[]
{
	1.0,		// meters
	0.001,      // km 
	3.28084,    // feet
	1.093613,   // yards
	0.0006213711922373339,	// miles
};

const double LengthUnit::UNITS_TO_METERS[] =
{
	1.0,			// meters
	1.0 / 0.001,    // km
	1.0 / 3.28084,  // feet
	1.0 / 1.093613, // yards
	1.0 / 0.0006213711922373339,   // miles
};
