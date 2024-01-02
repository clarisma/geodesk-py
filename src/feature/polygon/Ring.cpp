// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "Ring.h"
#include "PointInPolygon.h"


GEOSCoordSequence* Polygonizer::Ring::createCoordSequence(GEOSContextHandle_t context)
{
    GEOSCoordSequence* coordSeq = GEOSCoordSeq_create_r(context, vertexCount_, 2);
    if (coordSeq)
    {
        Segment* seg = firstSegment_;
        Coordinate first = seg->backward ? seg->coords[seg->vertexCount - 1] : seg->coords[0];
        GEOSCoordSeq_setXY_r(context, coordSeq, 0, first.x, first.y);
        int pos = 1;
        do
        {
            seg->copyTo(context, coordSeq, pos);
            pos += seg->vertexCount - 1;
            seg = seg->next;
        }
        while (seg);
        assert(pos == vertexCount_);
    }
    return coordSeq;
}

GEOSGeometry* Polygonizer::Ring::createLinearRing(GEOSContextHandle_t context)
{
    GEOSCoordSequence* seq = createCoordSequence(context);
    return GEOSGeom_createLinearRing_r(context, seq);
}

// TODO: error handling, check for null returns (C API does not throw)
GEOSGeometry* Polygonizer::Ring::createPolygon(GEOSContextHandle_t context, Arena& arena)
{
    GEOSGeometry** holes;
    int holeCount;
    if (firstInner_)
    {
        holeCount = firstInner_->number_;
        holes = arena.allocArray<GEOSGeometry*>(holeCount);
        Ring* inner = firstInner_;
        for (int i = 0; i < holeCount; i++)
        {
            holes[i] = inner->createLinearRing(context);
            inner = inner->next();
        }
    }
    else
    {
        holeCount = 0;
        holes = nullptr;
    }

    GEOSGeometry* shell = createLinearRing(context);
    return GEOSGeom_createPolygon_r(context, shell, holes, holeCount);
}


void Polygonizer::Ring::calculateBounds()
{
    Segment* p = firstSegment_;
    do
    {
        bounds_.expandToIncludeSimple(p->bounds());
        p = p->next;
    }
    while (p);
}


PointInPolygon::Location Polygonizer::Ring::locateCoordinate(Coordinate c)
{
    PointInPolygon tester(c);
    Segment* seg = firstSegment_;
    do
    {
        if (tester.testAgainstWay(seg->way)) return PointInPolygon::Location::BOUNDARY;
        seg = seg->next;
    }
    while (seg);
    return tester.isInside() ? PointInPolygon::Location::INSIDE : PointInPolygon::Location::OUTSIDE;
}


bool Polygonizer::Ring::contains(Ring* potentialInner)
{
    // An inner ring may touch an outer in one point, which doesn't necessarily
    // mean that it lies within the outer. Therefore, if the first point checked
    // lies on the boundary, we check a second point

    Coordinate* coords = potentialInner->firstSegment_->coords;
    PointInPolygon::Location loc = locateCoordinate(coords[0]);
    if (loc == PointInPolygon::Location::INSIDE) return true;
    if (loc != PointInPolygon::Location::BOUNDARY) return false;
    return locateCoordinate(coords[1]) == PointInPolygon::Location::INSIDE;
}
