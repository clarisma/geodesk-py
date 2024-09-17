// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "feature/WayCoordinateIterator.h"
#include "feature/WayPtr.h"
#include "MonotoneChain.h"
#include "geom/Coordinate.h"

// TODO: Make this a template: Source, Iter
//  template<typename Source, typename Iter>
class WaySlicer
{
public:
	WaySlicer(WayPtr way);
	bool hasMore() const { return hasMore_; }
	void slice(MonotoneChain* chain, int maxVertexes);

private:
	// keep this order!
	WayCoordinateIterator iter_;
	Coordinate first_;
	Coordinate second_;
	bool hasMore_;
};
