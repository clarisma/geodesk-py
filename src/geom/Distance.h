// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cmath>
#include <cstdint>
#include "Coordinate.h"

class Distance
{
public:
    /**
     * Calculates the square of the distance between two points
     * (x1,y1) and (x2, y2).
     *
     * @return distance (in units) squared.
     */
    static double pointsSquared(double x1, double y1, double x2, double y2);

    /**
     * Calculates the square of the distance between a line segment
     * [(x1,y1), (x2, y2)] and a point (px,py)
     *
     * @return distance (in units) squared.
     */
    static double pointSegmentSquared(double x1, double y1, double x2, double y2, double px, double py);
};

