// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "SpatialFilter.h"
#include "geom/Box.h"

class ContainsPointFilter : public SpatialFilter
{
public:
	ContainsPointFilter(Coordinate pt) : SpatialFilter(Box(pt)), point_(pt)	{}

	bool accept(FeatureStore* store, const FeaturePtr feature, FastFilterHint fast) const override;

private:
	Coordinate point_;
};
