// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <Python.h>
#include <vector>

class FeatureStore;
class PyQuery;

class PyQueryFinalizer
{
public:
    PyObject_HEAD
    std::vector<PyQuery*> queries;
    
    static PyTypeObject TYPE;

    static PyQueryFinalizer* create();
    static PyObject* call(PyQueryFinalizer* self, PyObject* args, PyObject* kwargs);
    static void dealloc(PyQueryFinalizer* self);
    static PyObject* str(PyQueryFinalizer* self);

    void track(PyQuery* query);
    void attemptCompletion();
    void awaitCompletion(FeatureStore* store);

private:
    static void dispose(PyQuery* query);

    /**
     * Returns a new reference to "gc.callbacks", or NULL if an exception occurred.
     */
    static PyObject* getGarbageCollectorCallbacks();
};







