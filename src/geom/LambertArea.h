// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "geom/polygon/Polygonizer.h"
#include "project/Lambert.h"
#include "project/Sinusoidal.h"
#include "geom/Mercator.h"

class LambertArea
{
protected:
    struct ProjectedCoordinate
    {
        double x;
        double y;
    };

public:
    /**
     * Returns the signed area (in squared meters)
     * of the given way; assumes that way is an area.
     * (This function is useful for getting the winding order)
     */
    static double signedOfWay(WayPtr way);
    static double signedOfRing(const Polygonizer::Ring* ring);
    static double ofWay(const WayPtr way)
	{
        return std::abs(signedOfWay(way));
	}
    static double ofRing(const Polygonizer::Ring* ring)
    {
        return std::abs(signedOfRing(ring));
    }

    /**
     * Returns the area (in square meters) of the given relation.
     * Assumes that the relation is an area.
     */
    static double ofRelation(FeatureStore* store, RelationPtr relation);

    static ProjectedCoordinate project(Coordinate c)
    {
        double lat = Mercator::latFromY(c.y);
        return ProjectedCoordinate
        {
            Sinusoidal::xFromLonLat(Mercator::lonFromX(c.x), lat),
            Sinusoidal::yFromLat(lat)
        };
    }

    /*
    static ProjectedCoordinate project(Coordinate c)
    {
        return ProjectedCoordinate
        {
            Lambert::xFromLon(Mercator::lonFromX(c.x)),
            Lambert::yFromLat(Mercator::latFromY(c.y))
        };
    }
    */
        
	template<typename Iter>
	static double signedOfAbstractRing(Iter& iter)
	{
        ProjectedCoordinate prev = project(iter.next());
        ProjectedCoordinate middle = project(iter.next());
        double sum = 0.0;
        double x0 = prev.x;
        for (int count = iter.coordinatesRemaining(); count > 0; count--)
        {
            ProjectedCoordinate next = project(iter.next());
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
