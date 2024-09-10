// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "LambertArea.h"
#include "feature/polygon/RingCoordinateIterator.h"


double LambertArea::signedOfWay(const WayPtr way)
{
    assert(way.isArea());
    WayCoordinateIterator iter;
    iter.start(way, FeatureFlags::AREA);
    return signedOfAbstractRing(iter);
}


double LambertArea::signedOfRing(const Polygonizer::Ring* ring)
{
    RingCoordinateIterator iter(ring);
    return signedOfAbstractRing(iter);
}


double LambertArea::ofRelation(FeatureStore* store, const RelationPtr relation)
{
    assert(relation.isArea());
    double totalArea = 0;

    Polygonizer polygonizer;
    polygonizer.createRings(store, relation);
    const Polygonizer::Ring* ring = polygonizer.outerRings();
    while (ring)
    {
        totalArea += LambertArea::ofRing(ring);
        ring = ring->next();
    }
    ring = polygonizer.innerRings();
    while (ring)
    {
        totalArea -= LambertArea::ofRing(ring);
        ring = ring->next();
    }
    return totalArea;
}
