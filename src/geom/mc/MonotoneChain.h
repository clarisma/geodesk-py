// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "geom/Box.h"

class MonotoneChain
{
public:
    void initLineSegment(Coordinate start, Coordinate end)
    {
        coordCount = 2;
        coords[0] = start;
        coords[1] = end;
    }

    /**
     * Finds the first segment in the given monotone chain for which y <= segment.endY.
     * The function assumes that y is greater or equal to the lowest Y of the chain,
     * and <= the highest Y of the chain (i.e. it falls into the Y-interval of the chain).
     *
     * @return pointer to the segment's start vertex
     */
    const Coordinate* findSegmentForY(int32_t y) const;

    /**
     * Checks if this chain intersects another. This fucntion assumes that a 
     * preliminary check already established that their bounding boxes intersect.
     */
    bool intersects(const MonotoneChain* other) const;
    
    /**
     * Copies this chain to another, ensuring that Y-coordinates are increasing.
     */
    void copyNormalized(MonotoneChain* dest) const;

    bool isNormalized() const { return coords[1].y >= coords[0].y; }
    void reverse();
    void normalize() { if (!isNormalized()) reverse(); }
    int vertexCount() const { return coordCount; }
    size_t storageSize() const { return coordCount * sizeof(Coordinate) + sizeof(coordCount); }
    static constexpr size_t storageSize(int vertexCount) { return vertexCount * sizeof(Coordinate) + sizeof(coordCount); }

    // void createFromWay(Coordinate start, int wayFlags, WayCoordinateIterator& iter, int maxVertexes);

    // TODO: could be fatser for normalized MC
    Box bounds() const
    {
        return Box::normalizedSimple(coords[0], coords[coordCount - 1]);
    }

    Coordinate first() const { return coords[0]; }
    Coordinate last() const  { return coords[coordCount-1]; }

private:
    void copyCoordinates(Coordinate* dest, int direction) const;

	int32_t coordCount;
	Coordinate coords[1];		// variable-length

    friend class WaySlicer;
    friend class CoordSequenceSlicer;
};
