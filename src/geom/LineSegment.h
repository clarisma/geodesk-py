// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "geom/Coordinate.h"

class LineSegment
{
public:
    LineSegment(Coordinate start_, Coordinate end_) :
        start(start_), end(end_) {}

    bool operator== (const LineSegment& other) const
    {
        return start == other.start && end == other.end;
    }

    void reverse()
    {
        std::swap(start, end);
    }

    void normalize()
    {
        if (start.y >= end.y)
        {
            if(start.y > end.y || start.x > end.x) reverse();
        }
    }

    static int orientation(double x1, double y1, double x2, double y2, double px, double py);
    static int orientation(Coordinate start, Coordinate end, Coordinate pt)
    {
        return orientation(start.x, start.y, end.x, end.y, pt.x, pt.y);
    }

    static bool linesIntersect(double x1, double y1, double x2, double y2,
        double x3, double y3, double x4, double y4);
    static bool linesIntersect(Coordinate start1, Coordinate end1, Coordinate start2, Coordinate end2)
    {
        return linesIntersect(
            start1.x, start1.y, end1.x, end1.y,
            start2.x, start2.y, end2.x, end2.y);
    }

    Coordinate start;
    Coordinate end;
};

// Make LineSegment hashable

namespace std
{
    template<>
    struct hash<LineSegment>
    {
        size_t operator()(const LineSegment& seg) const
        {
            return (hash<Coordinate>()(seg.start) * 37) ^ hash<Coordinate>()(seg.end);
        }
    };
}