// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cmath>
#include <geos_c.h>
#include "geom/Coordinate.h"

class GeosCoordinateIterator
{
public:
	GeosCoordinateIterator(GEOSContextHandle_t context, const GEOSCoordSequence* coords) :
		context_(context),
		coords_(coords),
		coordCount_(0),
		current_(0)
	{
		GEOSCoordSeq_getSize_r(context, coords, &coordCount_);
		// If call fails, coordCount_ remains 0 (initialized in list)
		// TODO: what can cause failure & how to handle?
	}

	int coordinatesRemaining() const { return coordCount_ - current_; }
	Coordinate next()
	{
		double x = 0;
		double y = 0;
		GEOSCoordSeq_getXY_r(context_, coords_, current_++, &x, &y);
		return Coordinate((int32_t)std::round(x), (int32_t)std::round(y));
	}

private:
	GEOSContextHandle_t context_;
	const GEOSCoordSequence* coords_;
	unsigned int coordCount_;
	unsigned int current_;
};


