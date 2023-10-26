// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "algorithms.h"


// TODO: need to use a counter if we treat vertex intersections as "one-half"
// add 2 for proper intersection, 1 for vertex (^ won't work here)

// TODO: if we use a counter, make it unsigned, because signed overflow
// is technically undefined behavior


int PointInPolygon::testFast(int32_t* coords, int len, double cx, double cy)
{
    int odd = 0;
    len -= 2;
    for (int i = 0; i < len; i += 2)
    {
        double x1 = coords[i];
        double y1 = coords[i + 1];
        double x2 = coords[i + 2];
        double y2 = coords[i + 3];

        // Skips vertex check for ~20% speedup

        if (((y1 <= cy) && (y2 > cy))     // upward crossing
            || ((y1 > cy) && (y2 <= cy))) // downward crossing
        {
            // compute edge-ray intersect x-coordinate
            double vt = (cy - y1) / (y2 - y1);
            if (cx < x1 + vt * (x2 - x1)) // P.x < intersect
            {
                odd ^= 1;
            }
        }
    }
    return odd;
}
