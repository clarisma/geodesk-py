// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cmath>
#include <cstdint>
#include "Coordinate.h"

namespace PointInPolygon
{
    /**
     * Fast but non-robust point-in-polygon test using the ray-crossing method.
     * Points located on a polygon edge (or very close to it) may or may not
     * be considered "inside." Vertexes, however, are always identified correctly.
     *
     * This test can be applied to multiple line strings of the polygon
     * in succession. In that case, the result of each test must be XOR'd
     * with the previous results.
     *
     * @param coords    pairs of x/y coordinates that form a polygon
     *                  or part thereof
     * @param len       number of coordinate values (counted individually, not pairs)
     * @param cx        the X-coordinate to test
     * @param cy        the Y-coordinate to test
     * @return          0 if even number of edges are crossed ("not inside")
     *                  1 if odd number of edges are crossed ("inside")
     */
	extern int testFast(int32_t* coords, int len, double cx, double cy);
}


