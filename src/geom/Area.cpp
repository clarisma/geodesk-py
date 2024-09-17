// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "Area.h"
#include "geom/polygon/RingCoordinateIterator.h"

/*
// see http://en.wikipedia.org/wiki/Shoelace_formula
double Area::ofWaySigned(const WayRef way)
{
    assert(way.flags() & FeatureFlags::AREA);
    WayCoordinateIterator iter;
    iter.start(way, FeatureFlags::AREA);
    Coordinate prev = iter.next();
    Coordinate middle = iter.next();
    double sum = 0.0;
    double x0 = prev.x;
    for(;;)
    {
        Coordinate next = iter.next();
        if (next.isNull()) break;
        double x = middle.x - x0;
        double y1 = next.y;
        double y2 = prev.y;
        sum += x * (y2 - y1);
        prev = middle;
        middle = next;
    }
    return sum / 2.0;
}
*/


double Area::signedMercatorOfWay(const WayPtr way)
{
    assert(way.isArea());
    WayCoordinateIterator iter;
    iter.start(way, FeatureFlags::AREA);
    return signedMercatorOfAbstractRing(iter);
}


double Area::signedMercatorOfRing(const Polygonizer::Ring* ring)
{
    RingCoordinateIterator iter(ring);
    return signedMercatorOfAbstractRing(iter);
}


double Area::ofRelation(FeatureStore* store, const RelationPtr relation)
{
    assert(relation.isArea());
    int32_t avgY = Math::avg(relation.minY(), relation.maxY());
    double scale = Mercator::metersPerUnitAtY(avgY);
    scale *= scale;     // squared for square meters
    double totalArea = 0;

    Polygonizer polygonizer;
    polygonizer.createRings(store, relation);
    const Polygonizer::Ring* ring = polygonizer.outerRings();
    while (ring)
    {
        totalArea += mercatorOfRing(ring) * scale;
        ring = ring->next();
    }
    ring = polygonizer.innerRings();
    while (ring)
    {
        totalArea -= mercatorOfRing(ring) * scale;
        ring = ring->next();
    }

    // TODO: could apply scale at end; but we may also calculate
    // scale for each ring separately?

    return totalArea;
}
