// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cmath>
#include "Coordinate.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Sinusoidal
{
    constexpr double EARTH_RADIUS = 6371000; // in meters, assuming spherical Earth

    inline double xFromLonLat(double lon, double lat)
    {
        return EARTH_RADIUS * lon * M_PI / 180 * std::cos(lat * M_PI / 180);
    }

    inline double yFromLat(double lat)
    {
        return EARTH_RADIUS * lat * M_PI / 180;
    }
}