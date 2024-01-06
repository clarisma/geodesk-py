// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "feature/Way.h"
#include "feature/polygon/Polygonizer.h"
#include "Mercator.h"

// TODO: units are squared-mercator-units, not square meters
class Area
{
public:
    /**
     * Returns the signed area (in squared Mercator units)
     * of the given way; assumes that way is an area.
     * (This function is useful for getting the winding order)
     */
    static double signedMercatorOfWay(const WayRef way);
    static double signedMercatorOfRing(const Polygonizer::Ring* ring);
    static double ofWay(const WayRef way)
	{
        int32_t avgY = (static_cast<int64_t>(way.minY()) + way.maxY()) / 2;
        double scale = Mercator::metersPerUnitAtY(avgY);
		return std::abs(signedMercatorOfWay(way)) * scale * scale;
	}
    static double mercatorOfRing(const Polygonizer::Ring* ring)
    {
        return std::abs(signedMercatorOfRing(ring));
    }

    /**
     * Returns the area (in square meters) of the given relation.
     * Assumes that the relation is an area.
     */
    static double ofRelation(FeatureStore* store, const RelationRef relation);

	template<typename Iter>
	static double signedMercatorOfAbstractRing(Iter& iter)
	{
        Coordinate prev = iter.next();
        Coordinate middle = iter.next();
        double sum = 0.0;
        double x0 = prev.x;
        for (int count = iter.coordinatesRemaining(); count > 0; count--)
        {
            Coordinate next = iter.next();
            double x = middle.x - x0;
            double y1 = next.y;
            double y2 = prev.y;
            sum += x * (y2 - y1);
            prev = middle;
            middle = next;
        }
        return sum / 2.0;
	}
};
