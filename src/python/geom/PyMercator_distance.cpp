// Copyright (c) 2026 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyMercator.h"
#include <geodesk/geom/Distance.h>
#include <geodesk/geom/SpatialUnit.h>
#include "ShapeHolder.h"
#include "python/Environment.h"


int PyMercator::unitsFromArg(const char* arg, bool forArea)
{
    if (!arg) return SpatialUnit::METERS;
    int units = SpatialUnit::unitFromString(arg, false);
    if (units < 0)  [[unlikely]]
    {
        PyErr_Format(PyExc_TypeError, "Units must be %s", forArea ?
            SpatialUnit::AREA_UNITS : SpatialUnit::LENGTH_UNITS);
        return -1;
    }
    return units;
}

PyObject* PyMercator::distance (PyObject* self, PyObject* args, PyObject* kwargs)
{
    static const char *const KEYWORDS[] = { "", "", "units", nullptr};

    ShapeHolder g1;
    ShapeHolder g2;
    GEOSContextHandle_t ctx = Environment::get().getGeosContext();
    if (!ctx) return NULL;

    PyObject* obj1;
    PyObject* obj2;
    const char* unitsArg = nullptr;

    int res;
    if (!PyModule_CheckExact(self))
    {
        obj1 = self;
        res = PyArg_ParseTupleAndKeywords(
            args, kwargs, "O|s:distance",
            const_cast<char **>(&KEYWORDS[1]), &obj2, &unitsArg);
    }
    else
    {
        res = PyArg_ParseTupleAndKeywords(
            args, kwargs, "OO|s:distance",
            const_cast<char **>(KEYWORDS), &obj1, &obj2, &unitsArg);
    }
    if (res == 0) return NULL;
    int units = unitsFromArg(unitsArg, false);
    if (units < 0) return NULL;
    if (!g1.setFromObject(obj1, ctx)) return NULL;
    if (!g2.setFromObject(obj2, ctx)) return NULL;

    double x1, y1, x2, y2;
    if (g1.isCoordinate() && g2.isCoordinate())
    {
        Coordinate c1 = g1.coordinate();
        x1 = c1.x;
        y1 = c1.y;
        Coordinate c2 = g2.coordinate();
        x2 = c2.x;
        y2 = c2.y;
    }
    else
    {
        GEOSCoordSequence *nearestPoints = GEOSNearestPoints_r
            (ctx, g1.asGeometry(), g2.asGeometry());
        if (!nearestPoints) [[unlikely]]
        {
            return Python::geosError("distance");
        }
        bool success =
            GEOSCoordSeq_getXY_r(ctx, nearestPoints, 0, &x1, &y1) &&
            GEOSCoordSeq_getXY_r(ctx, nearestPoints, 1, &x2, &y2);
        GEOSCoordSeq_destroy(nearestPoints);
        if (!success)  [[unlikely]]
        {
            return Python::geosError("distance");
        }
    }

    double d = Distance::metersBetween(x1,y1,x2,y2);
    d = SpatialUnit::fromMeters(d, units);
    return PyFloat_FromDouble(d);
}

