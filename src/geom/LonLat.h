// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "geom/Mercator.h"
#include <common/text/Format.h>

class LonLat
{
public:
	LonLat(Coordinate c) :
		lon(Mercator::lonFromX(c.x)),
		lat(Mercator::latFromY(c.y)) {}

	void format(char* buf) const
	{
		Format::unsafe(buf, "%.7f,%.7f", lon, lat);
	}

	std::string toString() const
	{
		char buf[32];
		format(buf);
		return std::string(buf);
	}

	double lon;
	double lat;
};
