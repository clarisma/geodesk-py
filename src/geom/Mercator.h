// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cmath>
#include <common/util/log.h>
#include <common/util/math.h>
#include "Coordinate.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Mercator
{
	constexpr double MAP_WIDTH = 4'294'967'294.9999;
	constexpr double EARTH_CIRCUMFERENCE = 40'075'016.68558;
    constexpr double MIN_LAT = -85.0511288;
    constexpr double MAX_LAT =  85.0511287;
    constexpr int32_t MIN_LAT_100ND = -850511288;
    constexpr int32_t MAX_LAT_100ND =  850511287;
    constexpr int32_t MIN_Y = INT_MIN;
    constexpr int32_t MAX_Y = INT_MAX - 1;

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

    inline double lonFromX(double x)
    {
        return x * 360.0 / MAP_WIDTH;
    }

    inline double latFromY(double y)
    {
        return std::atan(std::exp(y * M_PI * 2.0 / MAP_WIDTH)) * 360.0 / M_PI - 90.0;
    }
    
    inline double scale(double y)
    {
        /*
        for (double lat = 0; lat <= 86; lat++)
        {
            LOG("new scale at %f lat = %f", lat, 1 / std::cos(lat * M_PI / 180.0));
            LOG("old scale at %f lat = %f", lat, std::cosh(yFromLat(lat) * 2.0 * M_PI / MAP_WIDTH));
        }
        double latMax = 85.0511;
        LOG("new scale at %f lat = %f", latMax, 1 / std::cos(latMax * M_PI / 180.0));
        LOG("old scale at %f lat = %f", latMax, std::cosh(yFromLat(latMax) * 2.0 * M_PI / MAP_WIDTH));
        */

        // Re Issue #33:
        // The original code is fine, equivalent to this (more expensive) formula
        // up to Mercator maximum:
        // return 1 / std::cos(latFromY(y) * M_PI / 180.0);

        return std::cosh(y * 2.0 * M_PI / MAP_WIDTH);
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
        return d * metersPerUnitAtY(static_cast<double>(Math::avg(p1.y, p2.y)));
    }
}
