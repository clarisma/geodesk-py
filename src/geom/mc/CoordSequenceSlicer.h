// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <geos_c.h>
#include "MonotoneChain.h"
#include "geom/Coordinate.h"
#include "geom/geos/GeosCoordinateIterator.h"

// TODO: Make this a template: Source, Iter
//  template<typename Source, typename Iter>
class CoordSequenceSlicer
{
public:
	CoordSequenceSlicer(GEOSContextHandle_t context, const GEOSCoordSequence* coords);
	bool hasMore() const { return hasMore_; }
	void slice(MonotoneChain* chain, int maxVertexes);

private:
	// keep this order!
	GeosCoordinateIterator iter_;
	Coordinate first_;
	Coordinate second_;
	bool hasMore_;
};
