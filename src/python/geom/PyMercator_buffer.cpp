// Copyright (c) 2026 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyMercator.h"
#include <geodesk/geom/Distance.h>
#include "ShapeHolder.h"
#include "python/Environment.h"


PyObject* PyMercator::bufferArgsError(bool isMethod)
{
    PyErr_SetString(PyExc_TypeError,
        isMethod ?
            "Expected buffer(distance) or buffer(*units*=distance)" :
            "Expected buffer(geom, distance) or buffer(geom, *units*=distance)");
    return NULL;
}


PyObject* PyMercator::buffer (PyObject* self, PyObject* args, PyObject* kwargs)
{
    Environment& env = Environment::get();
    GEOSContextHandle_t ctx = env.getGeosContext();
    if (!ctx) return NULL;

    bool isMethod = !PyModule_CheckExact(self);
    Py_ssize_t argCount = PyTuple_GET_SIZE(args);
    if (argCount > (2 - isMethod))
    {
        return bufferArgsError(isMethod);
    }

    PyObject* geomObj;
    if (isMethod)
    {
        geomObj = self;
    }
    else
    {
        if (argCount < 1) return bufferArgsError(false);
        geomObj = PyTuple_GET_ITEM(args, 0);
    }
    PyObject* distanceObj = nullptr;
    if (argCount > 1 - isMethod)
    {
        distanceObj = PyTuple_GET_ITEM(args, 1 - isMethod);
    }

    ShapeHolder geom;
    if (!geom.setFromObject(geomObj, ctx)) return NULL;

    if (!distanceObj) return bufferArgsError(isMethod);

    // TODO: units

    double d = PyFloat_AsDouble(distanceObj);
    if (d == -1.0 && PyErr_Occurred()) return NULL;

    int32_t y = geom.asCoordinate().y;
    d = Mercator::unitsFromMeters(d, y);

    GEOSGeometry* buffered =
        GEOSBuffer_r(ctx, geom.asGeometry(), d, 8);
        // 8 quadsegs by default
    if (!buffered)
    {
        PyErr_SetString(PyExc_RuntimeError,
            "GEOS buffer operation failed");
        return NULL;
    }
    return env.buildShapelyGeometry(buffered);
}


