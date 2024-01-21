// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#pragma once

#include "query/Query.h"


// TODO: 
// Create a PyQueryCleaner which tracks queries that are no longer actively
// referenced, but still have asynchronously running tasks
// We must defer deallocation until all tasks complete
// One PyQueryCleaner exists per FeatureStore
// Also: When running PyQueryCleaner, must increase own refcount, becaue
// once the last deferred PyQuery is deallocated, FeatureStore may also be deallocated,
// which in turn would free the PQC itself. Decrease refcount at end of run

class PyFeatures;

class PyQuery : public PyObject
{
public:
    PyFeatures* target;
    Query query;

    static PyTypeObject TYPE;

    static PyQuery* create(PyFeatures* features);
    static void dealloc(PyQuery* self);
    static PyObject* iter(PyQuery* self);
    static PyObject* next(PyQuery* self);
};

