// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cstdint>
#include <Python.h>
#include "geom/Coordinate.h"


class PyCoordinate : public PyObject
{
public:
    int32_t x;
    int32_t y;

    static PyTypeObject TYPE;
    static PyMethodDef METHODS[];
    static PySequenceMethods SEQUENCE_METHODS;

    static PyCoordinate* create(int32_t x, int32_t y);
    static PyCoordinate* create(Coordinate c) { return create(c.x, c.y); }
    // static PyObject* create(PyTypeObject* type, PyObject* args, PyObject* kwargs);
    static int init(PyCoordinate* self, PyObject* args, PyObject* kwds);

    static PyObject* dir(PyCoordinate* self, PyObject* unused);
    static PyObject* getattr(PyCoordinate* self, PyObject* name);
    static Py_hash_t hash(PyCoordinate* self);
    static PyObject* richcompare(PyCoordinate* self, PyObject* other, int op);
    static PyObject* str(PyCoordinate* self);

    // Sequence Methods

    static PyObject* item(PyCoordinate* self, Py_ssize_t index);
    static Py_ssize_t len(PyObject* self);

    // General coordinate conversion methods

    /**
     * Converts the given Python int or float to a double lon value.
     * Checks for range -90 <= val <= 90.
     * Returns -1.0 in case of error (with exception set)
     */
    static double latValue(PyObject* obj)
    {
        double v = PyFloat_AsDouble(obj);
        if (v < -90 || v > 90)
        {
            PyErr_SetString(PyExc_ValueError, "lat must be in range -90 to 90");
            return -1.0;
        }
        return v;
    }

    /**
     * Converts the given Python int or float to a double lon value.
     * Checks for range -180 <= val <= 180.
     * Returns -1.0 in case of error (with exception set)
     */
    static double lonValue(PyObject* obj)
    {
        double v = PyFloat_AsDouble(obj);
        if (v < -180 || v > 180)
        {
            PyErr_SetString(PyExc_ValueError, "lon must be in range -180 to 180");
            return -1.0;
        }
        return v;
    }

    Coordinate coordinate() const { return Coordinate(x, y); }
};

extern double getCoordValue(PyObject* seq, int index);
// TODO: in PyBox for now





