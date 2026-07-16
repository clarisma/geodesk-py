// Copyright (c) 2026 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyMercator.h"
#include <geodesk/geom/SpatialUnit.h>
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
    int units = SpatialUnit::METERS;
    if (kwargs)
    {
        PyObject *key = nullptr;
        PyObject *value = nullptr;
        Py_ssize_t position = 0;

        while (PyDict_Next(
            kwargs,
            &position,
            &key,
            &value))
        {
            Py_ssize_t keyLen;
            const char* keyStr = PyUnicode_AsUTF8AndSize(key, &keyLen);
            if (!keyStr) return NULL;
            int possibleUnits = SpatialUnit::unitFromString(
                {keyStr,static_cast<size_t>(keyLen)}, false);
            if (possibleUnits >= 0)
            {
                // check if distance already specified
                if (distanceObj) return bufferArgsError(isMethod);
                distanceObj = value;
                units = possibleUnits;
            }
        }
    }
    if (!distanceObj) return bufferArgsError(isMethod);

    double d = PyFloat_AsDouble(distanceObj);
    if (d == -1.0 && PyErr_Occurred()) return NULL;
    d = SpatialUnit::toMeters(d, units);
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


