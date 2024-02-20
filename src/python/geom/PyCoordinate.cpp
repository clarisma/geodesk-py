// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyCoordinate.h"
#include "python/Environment.h"

// TODO: Since coordinates can be compared to simple tuples, 
// should be hash(coord) == hash(tuple) if coord == tuple
// Right now, this is not the case!

// TODO: PySequence_Fast crashes if called on PyCoordinate
//  PyCoordinate cannot conform to the protocol (it does not contain a list of items), 
//  but the call should fail with an error message instead of crashing

PyCoordinate* PyCoordinate::create(int32_t x, int32_t y)
{
    PyCoordinate* self = (PyCoordinate*)(TYPE.tp_alloc(&TYPE, 0));
    if (self)
    {
        self->x = x;
        self->y = y;
    }
    return self;
}

/*
PyObject* PyCoordinate::create(PyObject* args, bool latFirst)
{
    Py_ssize_t argCount = PySequence_Length(args);
    if (argCount != 2)
    {
        PyErr_SetString(PyExc_TypeError, latFirst ? "Expected (lat,lon)" : "Expected (lon,lat)");
        return NULL;
    }
    PyObject* arg = PyTuple_GET_ITEM(args, latFirst ? 1 : 0);
    double lon = lonValue(arg);
    if (lon == -1.0 && PyErr_Occurred()) return NULL;
    arg = PyTuple_GET_ITEM(args, latFirst ? 0 : 1);
    double lat = latValue(arg);
    if (lat == -1.0 && PyErr_Occurred()) return NULL;
    
    return create(Mercator::xFromLon(lon), Mercator::yFromLat(lat));
}
*/

int PyCoordinate::init(PyCoordinate* self, PyObject* args, PyObject* kwargs)
{
    Py_ssize_t argCount = PySequence_Length(args);
    if(argCount > 0)
    {
        if (argCount == 2 && !kwargs)
        {
            double v = getCoordValue(args, 0);
            if (v == -1.0 && PyErr_Occurred())
            {
                PyErr_Clear();
            }
            else
            {
                self->x = (int32_t)v;
                v = getCoordValue(args, 1);
                if (v == -1.0 && PyErr_Occurred())
                {
                    PyErr_Clear();
                }
                else
                {
                    self->y = (int32_t)v;
                    return 0;
                }
            }
        }
        PyErr_SetString(PyExc_TypeError, "Expected (x,y)");
        return -1;
    }
    
    if (kwargs && PyDict_Check(kwargs))
    {
        PyObject* key;
        PyObject* value;
        Py_ssize_t pos = 0;

        while (PyDict_Next(kwargs, &pos, &key, &value))
        {
            // References are borrowed, don't dec refcount

            const char* name = PyUnicode_AsUTF8(key);
            if (!name) return -1;
            if (strcmp(name, "x") == 0)
            {
                double v = PyFloat_AsDouble(value);
                if (v == -1.0 && PyErr_Occurred()) return -1;
                self->x = (int32_t)round(v);
            }
            else if (strcmp(name, "y") == 0)
            {
                double v = PyFloat_AsDouble(value);
                if (v == -1.0 && PyErr_Occurred()) return -1;
                self->y = (int32_t)round(v);
            }
            else if (strcmp(name, "lon") == 0)
            {
                double v = lonValue(value);
                if (v == -1.0 && PyErr_Occurred()) return -1;
                self->x = (int32_t)Mercator::xFromLon(v);
            }
            else if (strcmp(name, "lat") == 0)
            {
                double v = latValue(value);
                if (v == -1.0 && PyErr_Occurred()) return -1;
                self->y = (int32_t)Mercator::yFromLat(v);
            }
            else
            {
                PyErr_Format(PyExc_TypeError, "'%s' is an invalid keyword argument for Coordinate()", name);
                return -1;
            }
        }
    }
    return 0;
}



PyObject* PyCoordinate::getattr(PyCoordinate* self, PyObject* name)
{
    // Convert the PyObject* name to a C string for comparison
    const char* attr = PyUnicode_AsUTF8(name);
    if (!attr) return NULL;  // Error was set by PyUnicode_AsUTF8 if it's NULL
    
    if (strcmp(attr, "x") == 0) return PyLong_FromLong(self->x);
    if (strcmp(attr, "y") == 0) return PyLong_FromLong(self->y);
    if (strcmp(attr, "lon") == 0) 
    {
        return niceLonFromX(self->x);
    }
    if (strcmp(attr, "lat") == 0) 
    {
        return niceLatFromY(self->y);
    }
    return PyObject_GenericGetAttr((PyObject*)self, name);
}

PyObject* PyCoordinate::dir(PyCoordinate* self, PyObject* /* unused */)
{
    return PyObject_Dir((PyObject*)self);
}

PyObject* PyCoordinate::str(PyCoordinate* self)
{
    return PyUnicode_FromFormat("(%d, %d)", self->x, self->y);
}

Py_ssize_t PyCoordinate::len(PyObject* self)
{
    return 2;
}

PyObject* PyCoordinate::item(PyCoordinate* self, Py_ssize_t index)
{
    int32_t value;
    if (index < 0 || index > 1) return raiseIndexOutOfRange(index);
    value = ((int32_t*)&self->x)[index];

    // Returning potential 3rd value doesn't work

    /*
    if (index < 0) return raiseIndexOutOfRange(index);
    if (index > 1)
    {
        if (index > 2) return raiseIndexOutOfRange(index);
        value = 0;      // requested 3rd ordinate (height) is simply set to 0
    }
    else
    {
        value = ((int32_t*)&self->x)[index];
    }
    */
    return PyLong_FromLong(value);
}

Py_hash_t PyCoordinate::hash(PyCoordinate* self)
{
    return (((Py_hash_t)self->y) << 32) | self->x;
}

PyObject* PyCoordinate::richcompare(PyCoordinate* self, PyObject* other, int op)
{
    if (Py_TYPE(other) == &TYPE)
    {
        PyCoordinate* c = (PyCoordinate*)other;
        switch (op)
        {
        case Py_EQ:
            if (self->x == c->x && self->y == c->y) Py_RETURN_TRUE;
            Py_RETURN_FALSE;
        case Py_NE:
            if (self->x != c->x || self->y != c->y) Py_RETURN_TRUE;
            Py_RETURN_FALSE;
        default:
            Py_RETURN_NOTIMPLEMENTED;
        }
    }
    else if (PySequence_Check(other))
    {
        if (PySequence_Length(other) == 2)
        {
            double x = getCoordValue(other, 0);
            if (x == -1.0 && PyErr_Occurred())
            {
                PyErr_Clear();
            }
            else
            {
                double y = getCoordValue(other, 1);
                if (y == -1.0 && PyErr_Occurred())
                {
                    PyErr_Clear();
                }
                else
                {
                    if (self->x == x && self->y == y && op==Py_EQ) Py_RETURN_TRUE;
                    Py_RETURN_FALSE;
                }
            }
        }
    }
    switch (op)
    {
    case Py_EQ:
        Py_RETURN_FALSE;
    case Py_NE:
        Py_RETURN_TRUE;
    default:
        Py_RETURN_NOTIMPLEMENTED;
    }
}

PyMethodDef PyCoordinate::METHODS[] =
{
    {"__dir__", (PyCFunction)dir, METH_NOARGS, NULL },
    { NULL, NULL, 0, NULL },
};

PySequenceMethods PyCoordinate::SEQUENCE_METHODS
{
    .sq_length = &len,
    .sq_item = (ssizeargfunc)item,
};


PyTypeObject PyCoordinate::TYPE =
{
    .tp_name = "geodesk.Coordinate",
    .tp_basicsize = sizeof(PyCoordinate),
    .tp_repr = (reprfunc)str,
    // tp_as_number 
    .tp_as_sequence = &SEQUENCE_METHODS,
    // tp_as_mapping 
    .tp_hash = (hashfunc)hash,
    // tp_call 
    .tp_str = (reprfunc)str,
    .tp_getattro = (getattrofunc)getattr,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = "Coordinate objects",
    .tp_richcompare = (richcmpfunc)richcompare,
    .tp_init = (initproc)init,
    .tp_new = PyType_GenericNew,
};


PyCoordinate::ConversionResult PyCoordinate::xFromLon(PyObject* obj)
{
    PyCoordinate::ConversionResult res;
    double lon;
    if (PyFloat_Check(obj))
    {
        lon = PyFloat_AS_DOUBLE(obj);
    }
    else
    {
        lon = PyFloat_AsDouble(obj);
        if (lon == -1.0 && PyErr_Occurred())
        {
            res.success = false;
            return res;
        }
    }
    if(lon < -180 || lon > 180)
    {
        PyErr_SetString(PyExc_ValueError, "lon must be in range -180 to 180");
        res.success = false;
        return res;
    }
    res.success = true;
    res.value = Mercator::xFromLon(lon);
    return res;
}

PyCoordinate::ConversionResult PyCoordinate::yFromLat(PyObject* obj)
{
    PyCoordinate::ConversionResult res;
    double lat;
    if (PyFloat_Check(obj))
    {
        lat = PyFloat_AS_DOUBLE(obj);
    }
    else
    {
        lat = PyFloat_AsDouble(obj);
        if (lat == -1.0 && PyErr_Occurred())
        {
            res.success = false;
            return res;
        }
    }
    if (lat < Mercator::MIN_LAT)
    {
        if (lat < -90)
        {
            PyErr_SetString(PyExc_ValueError, "lat must be in range -90 to 90");
            res.success = false;
            return res;
        }
        lat = Mercator::MIN_LAT;
    }
    else if (lat > Mercator::MAX_LAT)
    {
        if (lat > 90)
        {
            PyErr_SetString(PyExc_ValueError, "lat must be in range -90 to 90");
            res.success = false;
            return res;
        }
        lat = Mercator::MAX_LAT;
    }
    res.success = true;
    res.value = Mercator::yFromLat(lat);
    return res;
}


PyCoordinate* PyCoordinate::createSingleFromItems(PyObject** items, int n, bool latFirst)
{
    PyObject* lon = items[n + (latFirst ? 1 : 0)];
    PyObject* lat = items[n + (latFirst ? 0 : 1)];
    ConversionResult xRes = xFromLon(lon);
    if (!xRes.success) return NULL;
    ConversionResult yRes = yFromLat(lat);
    if (!yRes.success) return NULL;
    return create(xRes.value, yRes.value);
}

const char* PyCoordinate::ERR_EXPECTED_COORD_LIST =
    "Expected list of coordinate pairs";
const char* PyCoordinate::ERR_EXPECTED_COORD_PAIR =
    "Expected pair of coordinate values";

PyObject* PyCoordinate::createMultiFromTupleItems(PyObject** items, Py_ssize_t count, bool latFirst)
{
    PyObject* list = PyList_New(count);
    for (int i = 0; i < count; i++)
    {
        PyObject* item = items[i];
        PyObject* pair = PySequence_Fast(item, ERR_EXPECTED_COORD_PAIR);
        if (!pair)
        {
            Py_DECREF(list);
            return NULL;
        }
        Py_ssize_t pairSize = PySequence_Fast_GET_SIZE(pair);
        if (pairSize != 2)
        {
            Py_DECREF(pair);
            Py_DECREF(list);
            PyErr_SetString(PyExc_TypeError, ERR_EXPECTED_COORD_PAIR);
            return NULL;
        }
        PyObject** pairItems = PySequence_Fast_ITEMS(pair);
        PyCoordinate* coord = createSingleFromItems(pairItems, 0, latFirst);
        Py_DECREF(pair);
        if (!coord)
        {
            Py_DECREF(list);
            return NULL;
        }
        PyList_SET_ITEM(list, i, coord);    // steals ref to coord
    }
    return list;
}

PyObject* PyCoordinate::createMultiFromFastSequence(PyObject* seq, bool latFirst)
{
    Py_ssize_t count = PySequence_Fast_GET_SIZE(seq);
    if (count > 0)
    {
        PyObject** items = PySequence_Fast_ITEMS(seq);
        if (PySequence_Check(items[0]))
        {
            return createMultiFromTupleItems(items, count, latFirst);
        }
        else
        {
            // Flat sequence of coordinate values (must appear as pairs,
            // hence count must be even)
            if ((count % 2) == 0)
            {
                PyObject* list = PyList_New(count / 2);
                for (int i = 0; i < count; i += 2)
                {
                    PyCoordinate* coord = createSingleFromItems(items, i, latFirst);
                    if (!coord)
                    {
                        Py_DECREF(list);
                        return NULL;
                    }
                    PyList_SET_ITEM(list, i / 2, coord);    // steals ref to coord
                }
                return list;
            }
        }
    }
    PyErr_SetString(PyExc_TypeError, ERR_EXPECTED_COORD_LIST);
    return NULL;
}

PyObject* PyCoordinate::create(PyObject* args, bool latFirst)
{
    Py_ssize_t argCount = PyTuple_GET_SIZE(args);
    if (argCount == 2)
    {
        // This call is safe because args is always a tuple
        PyObject** items = PySequence_Fast_ITEMS(args);
        if (PySequence_Check(items[0]))
        {
            return createMultiFromTupleItems(items, argCount, latFirst);
        }
        return createSingleFromItems(items, 0, latFirst);
    }
    if (argCount == 1)
    {
        PyObject* arg = PyTuple_GET_ITEM(args, 0);
        PyObject* seq = PySequence_Fast(arg, ERR_EXPECTED_COORD_LIST);
        if (!seq) return NULL;
        PyObject* list = createMultiFromFastSequence(seq, latFirst);
        Py_DECREF(seq);
        return list;
    }
    // This will check if argument count is 0 and raise exception
    return createMultiFromFastSequence(args, latFirst);
}
