// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cmath>
#include "geom/Coordinate.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Lambert
{
    constexpr double EARTH_RADIUS = 6371000; // in meters, assuming spherical Earth
    // TODO: maybe use an ellipsoid model?
    // At 0 lat:     6,378,137
    // At +/-90 lat: 6,356,752
    inline double xFromLon(double lon)
    {
        return EARTH_RADIUS * lon * M_PI / 180;
    }

    inline double yFromLat(double lat)
    {
        return EARTH_RADIUS * std::sin(lat * M_PI / 180);
    }
}