// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "Environment.h"
#include <geos_c.h>
#include "python/query/PyQueryFinalizer.h"
#include "python/query/PyFeatures.h"
#include <common/util/log.h>

Environment::Environment() :
    geosContext_(nullptr),
    shapelyModule_(nullptr),
    shapelyApiFunctions_(nullptr),
    emptyFeatures_(nullptr),
    queryFinalizer_(nullptr),
    queryException_(nullptr)
{
    memset(stringConstants_, 0, sizeof(stringConstants_));
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
    Py_XDECREF(shapelyModule_);
    if (geosContext_) GEOS_finish_r(geosContext_);
    // Py_XDECREF(emptyFeatures_);
    Py_XDECREF(queryFinalizer_);
    // don't dispose of queryException_, it is tracked by the module itself
}

PyFeatures* Environment::getEmptyFeatures()
{
    if (!emptyFeatures_)
    {
        allMatcher_.addref();
        emptyFeatures_ = PyFeatures::createEmpty(&allMatcher_);
    }
    Py_INCREF(emptyFeatures_);
    return emptyFeatures_;
}


PyQueryFinalizer* Environment::getQueryFinalizer()
{
    if (!queryFinalizer_)
    {
        queryFinalizer_ = PyQueryFinalizer::create();
        if (!queryFinalizer_) return NULL;
    }
    Py_INCREF(queryFinalizer_);
    return queryFinalizer_;
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
    void** shapelyFunctions = getShapelyFunctions();
    if (!shapelyFunctions) return false;
    return ((ShapelyGetGEOSGeometryFunc)shapelyFunctions[1])(obj, pGeom);
}

Environment Environment::ENV;


