// Copyright (c) 2026 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyMercator.h"
#include <geodesk/geom/SpatialUnit.h>
#include "ShapeHolder.h"
#include "python/Environment.h"


/*

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
    int units = -1;
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
            std::string_view keyStrView(keyStr, keyLen);
            int possibleUnits = SpatialUnit::unitFromString(
                keyStrView, false);
            if (possibleUnits >= 0)
            {
                // check if distance already specified
                if (distanceObj || units >= 0) return bufferArgsError(isMethod);
                distanceObj = value;
                units = possibleUnits;
            }
            else if (keyStrView == "units")
            {
                Py_ssize_t valLen;
                const char* valStr = PyUnicode_AsUTF8(value);
                if (!valStr) return NULL;
                units = unitsFromArg(valStr, false);
                if (units < 0) return NULL;
            }
        }
    }
    units = units < 0 ? SpatialUnit::METERS : units;
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

*/


PyObject* PyMercator::buffer (PyObject* self, PyObject* args, PyObject* kwargs)
{
    static const char ARGS[] = "Od|s:buffer";
    static const char *const KEYWORDS[] = { "", "", "units", nullptr};

    Environment& env = Environment::get();
    GEOSContextHandle_t ctx = env.getGeosContext();
    if (!ctx) return NULL;

    PyObject* obj;
    double distance;
    const char* unitsArg = nullptr;

    int res;
    if (!PyModule_CheckExact(self))
    {
        obj = self;
        res = PyArg_ParseTupleAndKeywords(
            args, kwargs, &ARGS[1],
            const_cast<char **>(&KEYWORDS[1]), &distance, &unitsArg);
    }
    else
    {
        res = PyArg_ParseTupleAndKeywords(
            args, kwargs, ARGS,
            const_cast<char **>(KEYWORDS), &obj, &distance, &unitsArg);
    }
    if (res == 0) return NULL;

    int units = unitsFromArg(unitsArg, false);
    if (units < 0) return NULL;

    ShapeHolder geom;
    if (!geom.setFromObject(obj, ctx)) return NULL;

    distance = SpatialUnit::toMeters(distance, units);
    int32_t y = geom.asCoordinate().y;
    distance = Mercator::unitsFromMeters(distance, y);

    GEOSGeometry* buffered =
        GEOSBuffer_r(ctx, geom.asGeometry(), distance, 8);
        // 8 quadsegs by default
    if (!buffered) return Python::geosError("buffer");
    return env.buildShapelyGeometry(buffered);
}
