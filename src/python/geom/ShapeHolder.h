// Copyright (c) 2026 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <Python.h>
#include <geos_c.h>
#include <clarisma/util/TaggedPtr.h>
#include <geodesk/geom/GeometryBuilder.h>

using namespace geodesk;

class ShapeHolder
{
public:
    ~ShapeHolder()
    {
        if (geom_.flags()) GEOSGeom_destroy_r(context_, geom_.ptr());
    }
    bool setFromObject(PyObject* obj, GEOSContextHandle_t ctx);
    GEOSContextHandle_t context() const { return context_; }
    GEOSGeometry* geometry() const { return geom_.ptr(); }
    bool isCoordinate() const { return geom_.rawPtr() == nullptr; }
    Coordinate coordinate() const
    {
        assert(isCoordinate());
        return coord_;
    }

    GEOSGeometry* asGeometry();
    Coordinate asCoordinate();

private:
    GEOSContextHandle_t context_ = nullptr;
    clarisma::TaggedPtr<GEOSGeometry,1> geom_;
    Coordinate coord_;
};
