// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cmath>
#include "Coordinate.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Mercator
{
	constexpr double MAP_WIDTH = 4'294'967'294.9999;
	constexpr double EARTH_CIRCUMFERENCE = 40'075'016.68558;

    inline int32_t xFromLon(double lon)
    {
        return static_cast<int32_t>(round(MAP_WIDTH * lon / 360.0));
    }

    inline int xFromLon100nd(int lon)
    {
        return static_cast<int>(std::round(xFromLon(static_cast<double>(lon) / 10000000.0)));
    }

    inline int32_t yFromLat(double lat)
    {
        return static_cast<int32_t>(round(std::log(std::tan((lat + 90.0) 
            * M_PI / 360.0)) * (MAP_WIDTH / 2.0 / M_PI)));
    }

    inline int yFromLat100nd(int lat)
    {
        return static_cast<int>(std::round(yFromLat(static_cast<double>(lat) / 10000000.0)));
    }

    inline double scale(double y)
    {
        return std::cosh(y * 2.0 * M_PI / MAP_WIDTH);
    }

    inline double lonFromX(double x)
    {
        return x * 360.0 / MAP_WIDTH;
    }

    inline double latFromY(double y)
    {
        return std::atan(std::exp(y * M_PI * 2.0 / MAP_WIDTH)) * 360.0 / M_PI - 90.0;
    }
    
    static double metersPerUnitAtY(double y)
    {
        return EARTH_CIRCUMFERENCE / MAP_WIDTH / scale(y);
    }

    static inline double unitsFromMeters(double meters, double atY)
    {
        return meters * MAP_WIDTH / EARTH_CIRCUMFERENCE * scale(atY);
    }

    static inline double distance(Coordinate p1, Coordinate p2)
    {
        double xDelta = (double)p1.x - p2.x;
        double yDelta = (double)p1.y - p2.y;
        double d = sqrt(xDelta * xDelta + yDelta * yDelta);
        return d * metersPerUnitAtY((static_cast<int64_t>(p1.y) + p2.y) / 2);
    }
}
