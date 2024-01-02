// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <geos_c.h>
#include <geos/geom/Geometry.h>
#include "geom/Box.h"

class Geos
{
public:
	static Box getEnvelope(GEOSContextHandle_t context, GEOSGeometry* geom)
	{
		return Box(((geos::geom::Geometry*)geom)->getEnvelopeInternal());
	}

};
