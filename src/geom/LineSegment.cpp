// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "LineSegment.h"
#include <cmath>

/**
 * Determines the position of point (px,py) relative to the line segment [(x1,y1),(x2,y2)].
 * 
 * @returns   -1   point lies clockwise
 *             1   point lies counter-clockwise
 *             0   point lies on the line segment  
 * 
 */
int LineSegment::orientation(double x1, double y1, double x2, double y2, double px, double py)
{
    x2 -= x1;
    y2 -= y1;
    px -= x1;
    py -= y1;
    double ccw = px * y2 - py * x2;
    if (ccw == 0.0) 
    {
        ccw = px * x2 + py * y2;
        if (ccw > 0.0) 
        {
            px -= x2;
            py -= y2;
            ccw = px * x2 + py * y2;
            if (ccw < 0.0) ccw = 0.0;
        }
    }
    return (ccw < 0.0) ? -1 : ((ccw > 0.0) ? 1 : 0);
}


/**
 * Tests for intersection between two linestrings 
 * [(x1,y1),(x2,y2)] and [(x3,y3),(x4,y4)].
 */
bool LineSegment::linesIntersect(
    double x1, double y1,
    double x2, double y2,
    double x3, double y3,
    double x4, double y4)
{
    return ((orientation(x1, y1, x2, y2, x3, y3) *
        orientation(x1, y1, x2, y2, x4, y4) <= 0)
        && (orientation(x3, y3, x4, y4, x1, y1) *
            orientation(x3, y3, x4, y4, x2, y2) <= 0));
}

// TODO: need a function that returns exact relationship:
// -1 = no interaction
//  0 = line segments are colinear
//  1 = lines cross

// "Crossing" means both products are < 0
// "Touching" means either product == 0 ? 
