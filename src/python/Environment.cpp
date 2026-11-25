// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "Environment.h"
#include <geos_c.h>
#include "python/query/PyFeatures.h"
#include "python/feature/PyTags.h"
#include <clarisma/util/log.h>

Environment::Environment()
{
}

int Environment::init()
{
    for (int i = 0; i < STRING_CONSTANT_COUNT; i++)
    {
        PyObject* str = PyUnicode_FromString(STRING_CONSTANTS[i]);
        if (str == NULL) return -1;
        stringConstants_[i] = str;
    }
    return 0;
}

// keep in order
const char* Environment::STRING_CONSTANTS[STRING_CONSTANT_COUNT] =
{
    "",
    "node",
    "way",
    "relation",
    "invalid"
};

Environment::~Environment()
{
    // Py_XDECREF(shapelyModule_);
    // if (geosContext_) GEOS_finish_r(geosContext_);
    // Py_XDECREF(emptyFeatures_);
    // Py_XDECREF(emptyTags_);
    // don't dispose of queryException_, it is tracked by the module itself
}

void Environment::destroy()
{
    Py_XDECREF(shapelyModule_);
    if (geosContext_)
    {
        GEOS_finish_r(geosContext_);
        geosContext_ = nullptr;
    }
    Py_XDECREF(emptyFeatures_);
    Py_XDECREF(emptyTags_);
    // don't dispose of queryException_, it is tracked by the module itself
}

PyFeatures* Environment::getEmptyFeatures()
{
    if (!emptyFeatures_)    [[unlikely]]
    {
        emptyFeatures_ = PyFeatures::createEmpty();
    }
    return Python::newRef(emptyFeatures_);
}


PyTags* Environment::getEmptyTags()
{
    if (!emptyTags_)    [[unlikely]]
    {
        emptyTags_ = PyTags::createEmpty();
    }
    return Python::newRef(emptyTags_);
}


void Environment::clearAndLogException()
{
    if (PyErr_Occurred())
    {
        PyObject* ptype, * pvalue, * ptraceback;
        PyErr_Fetch(&ptype, &pvalue, &ptraceback);
        // PyErr_NormalizeException(&ptype, &pvalue, &ptraceback);

        // Ensure the exception value is a string
        PyObject* pStr = PyObject_Str(pvalue);
        const char* exceptionMessage = PyUnicode_AsUTF8(pStr);

        LOG("Exception: %s\n", exceptionMessage);

        Py_XDECREF(pStr);
        Py_XDECREF(ptype);
        Py_XDECREF(pvalue);
        Py_XDECREF(ptraceback);
    }

}


PyObject* Environment::raiseQueryException(const char* format, ...)
{
    // TODO: Need to expose class to user
    assert(queryException_);
    va_list vargs;
    va_start(vargs, format);
    PyErr_FormatV(queryException_, format, vargs);
    va_end(vargs);
    return NULL;
}


PyObject* Environment::buildShapelyGeometry(GEOSGeometry* geom)
{
    if (!geom)
    {
        // TODO: set up a GEOS error handler and use its result here?
        PyErr_SetString(PyExc_RuntimeError, "GEOS operation failed");
        return NULL;
    }
    void** shapelyFunctions = getShapelyFunctions();
    if (!shapelyFunctions) return NULL;
    return ((ShapelyCreateGeometryFunc)shapelyFunctions[0])(geom, getGeosContext());
}

bool Environment::getGeosGeometry(PyObject* obj, GEOSGeometry** pGeom)
{
    // Shapely returns nullptr instead of a valid GEOSGeometry
    // (while returning true) if handed `None` (cause for #93 and #78)

    void** shapelyFunctions = getShapelyFunctions();
    if (!shapelyFunctions) return false;
    if (!((ShapelyGetGEOSGeometryFunc)shapelyFunctions[1])(obj, pGeom)) [[unlikely]]
    {
        return false;
    }
    return *pGeom != nullptr;   // Treat null geometry is a failure
}

PyObject* Environment::createDatetimeModule()
{
    return PyImport_ImportModule("datetime");
}

PyObject* Environment::createDatetimeType()
{
    PyObject* module = datetimeModule();
    if (!module) return nullptr;
    return PyObject_GetAttrString(module, "datetime");
}

PyObject* Environment::createFromtimestampMethod()
{
    PyObject* datetime = datetimeType();
    if (!datetime) return nullptr;
    return PyObject_GetAttrString(datetime, "fromtimestamp");
}

PyObject* Environment::createUtcTimezone()
{
    PyObject* module = datetimeModule();
    if (!module) return nullptr;

    PyObject* timezone = PyObject_GetAttrString(module, "timezone");
    if (!timezone) return nullptr;

    PyObject* utc = PyObject_GetAttrString(timezone, "utc");
    Py_DECREF(timezone);
    return utc;
}

PyObject* Environment::datetime(clarisma::DateTime timestamp)
{
    PyObject* method = fromtimestampMethod();
    if (!method) return nullptr;

    PyObject* utc = utcTimezone();
    if (!utc) return nullptr;

    double seconds = static_cast<double>(timestamp) / 1000.0;
    return PyObject_CallFunction(method, "dO", seconds, utc);
}


Environment Environment::ENV;


