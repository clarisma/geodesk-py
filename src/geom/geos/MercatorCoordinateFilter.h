// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <geos/geom/CoordinateFilter.h>
#include <geos/geom/Geometry.h>
#include "geom/Mercator.h"

class ToMercatorCoordinateFilter : public geos::geom::CoordinateFilter 
{
public:
    void filter_rw(geos::geom::Coordinate* coord) const override 
    {
        coord->x = Mercator::xFromLon(coord->x);
        coord->y = Mercator::yFromLat(coord->y);
    }
};


class FromMercatorCoordinateFilter : public geos::geom::CoordinateFilter
{
public:
    void filter_rw(geos::geom::Coordinate* coord) const override
    {
        coord->x = Mercator::lonFromX(coord->x);
        coord->y = Mercator::latFromY(coord->y);
    }
};
