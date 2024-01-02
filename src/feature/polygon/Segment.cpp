// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "Segment.h"


void Polygonizer::Segment::copyTo(GEOSContextHandle_t context, GEOSCoordSequence* seq, int destPos) const
{
    // we skip first coordinate because it is already the end coordinate of previous
    // segment (for the first segment, the caller has to place the start coordinate)

    if (backward)
    {
        for (int i = vertexCount - 2; i >= 0; i--)
        {
            GEOSCoordSeq_setXY_r(context, seq, destPos++, coords[i].x, coords[i].y);
        }
    }
    else
    {
        for (int i = 1; i < vertexCount; i++)
        {
            GEOSCoordSeq_setXY_r(context, seq, destPos++, coords[i].x, coords[i].y);
        }
    }
}

Polygonizer::Segment* Polygonizer::Segment::createFragment(int start, int end, Arena& arena) const
{
    assert(start < end);
    int fragmentVertexCount = end - start;
    assert(fragmentVertexCount >= 2);
    Segment* fragment = arena.allocWithExplicitSize<Segment>(
        Segment::sizeWithVertexCount(fragmentVertexCount));
    fragment->next = nullptr;
    fragment->way = WayRef(nullptr);
    fragment->status = Segment::SEGMENT_UNASSIGNED;
    fragment->backward = false;
    fragment->vertexCount = fragmentVertexCount;
    Coordinate* p = fragment->coords;
    for (int i = start; i < end; i++)
    {
        *p++ = coords[i];
    }
    return fragment;
}

