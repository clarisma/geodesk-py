// Copyright (c) 2026 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyMercator.h"
#include <geodesk/geom/Distance.h>
#include "ShapeHolder.h"
#include "python/Environment.h"

PyObject* PyMercator::distance (PyObject* self, PyObject* args, PyObject* kwargs)
{
    static const char *const KEYWORDS[] = { "", "", "units", nullptr};

    ShapeHolder g1;
    ShapeHolder g2;
    GEOSContextHandle_t ctx = Environment::get().getGeosContext();
    if (!ctx) return NULL;

    PyObject* obj1;
    PyObject* obj2;
    PyObject* units;

    int res;
    if (!PyModule_CheckExact(self))
    {
        obj1 = self;
        res = PyArg_ParseTupleAndKeywords(
            args, kwargs, "O|O:distance",
            const_cast<char **>(&KEYWORDS[1]), &obj2);
    }
    else
    {
        res = PyArg_ParseTupleAndKeywords(
            args, kwargs, "OO|O:distance",
            const_cast<char **>(KEYWORDS), &obj1, &obj2);
    }
    if (res == 0) return NULL;
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
            PyErr_SetString(PyExc_RuntimeError,
                "Distance calculation failed");
            return NULL;
        }
        bool success =
            GEOSCoordSeq_getXY_r(ctx, nearestPoints, 0, &x1, &y1) &&
            GEOSCoordSeq_getXY_r(ctx, nearestPoints, 1, &x2, &y2);
        GEOSCoordSeq_destroy(nearestPoints);
        if (!success)
        {
            PyErr_SetString(PyExc_RuntimeError,
                "Distance calculation failed");
            return NULL;
        }
    }

    double d = Distance::metersBetween(x1,y1,x2,y2);
    // TODO: Adjust to requested units
    return PyFloat_FromDouble(d);
}

