// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <Python.h>
#include <geodesk/geom/Box.h>

using namespace geodesk;

class PyBox : public PyObject
{
public:
    Box box;

    static PyTypeObject TYPE;
    static PyNumberMethods NUMBER_METHODS;
    static PySequenceMethods SEQUENCE_METHODS;

    static PyObject* create(PyTypeObject* type, PyObject* args, PyObject* kwargs);
    static PyBox* create(const Box& bbox);
    static PyBox* create(int32_t x1, int32_t y1, int32_t x2, int32_t y2);
        
    static PyObject* getattr(PyBox* self, PyObject* name);
    static Py_hash_t hash(PyBox* self);
    static PyObject* repr(PyBox* self);
    static PyObject* richcompare(PyBox* self, PyObject* other, int op);
    static void dealloc(PyBox* self);
    static PyObject* str(PyBox* self);
    
    // Number Methods

    static int isTrue(PyBox* self);
    static int doAdd(Box& box, PyObject* other);
    static PyBox* add(PyBox* self, PyObject* other);
    static PyBox* iadd(PyBox* self, PyObject* other);
    static PyBox* intersection(PyBox* self, PyObject* other);

    // Sequence Methods

    static int contains(PyBox* self, PyObject* other);
    static PyObject* item(PyBox* self, Py_ssize_t index);
    static Py_ssize_t len(PyObject*);

    // Other methods

    static PyBox* buffer(PyBox* self, PyObject* args, PyObject* kwargs);
};







