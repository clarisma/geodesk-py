// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cstdint>
#include <Python.h>
#include <geodesk/geom/Coordinate.h>
#include <geodesk/geom/Mercator.h>

using namespace geodesk;

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
    static PyObject* create(PyObject* args, bool latFirst);
    static PyObject* createLonLat(PyObject* /* self */, PyObject* args)
    {
        return create(args, false);
    }
    static PyObject* createLatLon(PyObject* /* self */, PyObject* args)
    {
        return create(args, true);
    }
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

    static double precision7(double value)
    {
        static double factor = 10'000'000.0;
        return std::round(value * factor) / factor;
    }

    // TODO: Use Mercator::lon100ndFromX
    static PyObject* niceLonFromX(int32_t x)
    {
        return PyFloat_FromDouble(precision7(Mercator::lonFromX(x)));
    }

    static PyObject* niceLatFromY(int32_t y)
    {
        return PyFloat_FromDouble(precision7(Mercator::latFromY(y)));
    }

    struct ConversionResult
    {
        int32_t value;
        bool success;
    };

    static ConversionResult xFromLon(PyObject* obj);
    static ConversionResult yFromLat(PyObject* obj);

    /**
     * Creates a Coordinate from a sequence of coordinate values.
     * The following conditions must be met, or this fucntion will result
     * in undefined behavior:
     * 1. `seq` must be an object returned from PySequence_Fast
     * 2. index positions [n] and [n+1] must be valid
     * 
     * If the objects at [n] and [n+1] are not numeric (or are outside
     * the range of valid lon/lat values), this fucntion returns NULL with
     * an exception set.  
     */
    static PyCoordinate* createSingleFromItems(PyObject** items, int n, bool latFirst);
    
    /**
     * Creates a list of coordinates from the given sequence. Raises an 
     * exception if the sequence is empty.
     */
    static PyObject* createMultiFromFastSequence(PyObject* seq, bool latFirst);

    /**
     * Creates a list of coordinates from the given items. `items` must contain
     * at least one item. Raises an exception if any item does not represent
     * a tuple-like coordinate pair (each item can be any sequence, nor just tuple).
     */
    static PyObject* createMultiFromTupleItems(PyObject** items, Py_ssize_t, bool latFirst);

    Coordinate coordinate() const { return Coordinate(x, y); }

    static const char* ERR_EXPECTED_COORD_PAIR;
    static const char* ERR_EXPECTED_COORD_LIST;
};

extern double getCoordValue(PyObject* seq, int index);
// TODO: in PyBox for now





