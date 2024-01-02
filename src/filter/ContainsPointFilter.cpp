// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "ContainsPointFilter.h"
#include "feature/polygon/PointInPolygon.h"

bool ContainsPointFilter::accept(FeatureStore* store, FeatureRef feature, FastFilterHint /* ignored */) const
{
	if (feature.isArea())
	{
		if (feature.isWay())
		{
			PointInPolygon tester(point_);
			// point is on boundary or inside (odd number of crossings)
			return tester.testAgainstWay(WayRef(feature)) || tester.isInside();
		}
		else
		{
			PointInPolygon tester(point_);
			// point is on boundary or inside (odd number of crossings)
			return tester.testAgainstRelation(store, RelationRef(feature)) || tester.isInside();
		}
	}
	// TODO: lineal way, node, non-area relations
	return false;
}
