// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <Python.h>
#include "feature/FeaturePtr.h"

class PyTags
{
public:
    PyObject_HEAD
    FeatureStore* store;
    TagTablePtr tags;

    static PyTypeObject TYPE;
    static PyMappingMethods MAPPING_METHODS;

    static PyObject* create(FeatureStore* store, TagTablePtr tags);
    static void dealloc(PyTags* self);
    static Py_ssize_t len(PyTags* self);
    static PyObject* str(PyTags* self);
    static PyObject* iter(PyTags* self);
    static PyObject* subscript(PyTags* self, PyObject* keyObj);
};


class PyTagIterator
{
public:
    typedef PyObject* (*NextTagFunc)(PyTagIterator*);

    PyObject_HEAD
    FeatureStore* store;
    TagTablePtr tags;
    DataPtr current;
    NextTagFunc func;

    static PyTypeObject TYPE;

    static PyObject* create(FeatureStore* store, TagTablePtr tags);

private:
    static void dealloc(PyTagIterator* self);
    static PyObject* createTag(PyTagIterator* self, PyObject* key, uint64_t tagVal);
    static PyObject* next(PyTagIterator* self);
    static PyObject* nextGlobal(PyTagIterator* self);
    static PyObject* firstLocal(PyTagIterator* self);
    static PyObject* nextLocal(PyTagIterator* self);
    static PyObject* done(PyTagIterator* self);
};
