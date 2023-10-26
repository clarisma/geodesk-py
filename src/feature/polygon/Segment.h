// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "Polygonizer.h"
#include <common/alloc/Arena.h>

class Polygonizer::Segment
{
public:
    // The order matters for these constants
    enum
    {
        SEGMENT_UNASSIGNED = 0,
        SEGMENT_TENTATIVE = 1,
        SEGMENT_ASSIGNED = 2,
        SEGMENT_DANGLING = 3,
    };

    Segment* next;
    WayRef way;
    bool backward;
    uint8_t status;
    uint16_t vertexCount;      // TODO: 16 bits enough? No more than 64K per segment 
    Coordinate coords[1];      // variable length

    /*
    Coordinate start() const { return coords[0]; }
    Coordinate end()   const { return coords[vertexCount-1]; }
    */

    static size_t sizeWithVertexCount(int vertexCount)
    {
        return sizeof(Segment) + sizeof(Coordinate) * (vertexCount - 1);
        // -1 because the placeholder is already part of Segment's basic size
    }

    bool isClosed() const
    {
        return coords[0] == coords[vertexCount - 1];
    }

    Box bounds() const
    {
        return way.bounds();
    }

    void copyTo(GEOSContextHandle_t context, GEOSCoordSequence* seq, int destPos) const;
    Segment* createFragment(int start, int end, Arena& arena) const;
};


