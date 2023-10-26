// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "SpatialFilter.h"
#include "geom/Mercator.h"
#include "match/Matcher.h"

/**
 * Accepts all features that are within the given distance from the given point.
 */
class PointDistanceFilter : public SpatialFilter
{
public:
	PointDistanceFilter(double meters, Coordinate point);

	virtual bool accept(FeatureStore* store, FeatureRef feature, FastFilterHint fast) const;

private:
	bool segmentsWithinDistance(WayRef way, int areaFlag) const;
	bool isWithinDistance(WayRef way) const;
	bool isAreaWithinDistance(FeatureStore* store, RelationRef relation) const;
	bool areMembersWithinDistance(FeatureStore* store, RelationRef relation, RecursionGuard& guard) const;

	Coordinate point_;
	double distanceSquared_;
};