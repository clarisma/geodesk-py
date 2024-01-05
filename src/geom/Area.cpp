// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "Area.h"
#include "feature/polygon/RingCoordinateIterator.h"


double Area::signedOfWay(const WayRef way)
{
    assert(way.isArea());
    WayCoordinateIterator iter;
    iter.start(way, FeatureFlags::AREA);
    return signedOfAbstractRing(iter);
}


double Area::signedOfRing(const Polygonizer::Ring* ring)
{
    RingCoordinateIterator iter(ring);
    return signedOfAbstractRing(iter);
}


double Area::ofRelation(FeatureStore* store, const RelationRef relation)
{
    assert(relation.isArea());
    double totalArea = 0;

    Polygonizer polygonizer;
    polygonizer.createRings(store, relation);
    const Polygonizer::Ring* ring = polygonizer.outerRings();
    while (ring)
    {
        totalArea += Area::ofRing(ring);
        ring = ring->next();
    }
    ring = polygonizer.innerRings();
    while (ring)
    {
        totalArea -= Area::ofRing(ring);
        ring = ring->next();
    }
    return totalArea;
}
