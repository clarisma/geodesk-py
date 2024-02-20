// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyBox.h"
#include <geos_c.h>
#include "python/Environment.h"
#include "python/util/util.h"
#include "python/util/PyFastMethod.h"
#include "PyCoordinate.h"
#include "geom/Coordinate.h"
#include "geom/LengthUnit.h"
#include "geom/Mercator.h"
#include <common/util/log.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Envelope.h>

#include "PyBox_attr.cxx"


// define methods

/*

e
east
maxlat
maxx
maxy
maxlon
minlat
minlon
minx
miny
n
north
s
south
w
west

*/

// Generate the attribute lookup table using gperf:
// \sw\gperf\bin\gperf -t --lookup-function-name=PyBox_lookupAttr PyBox_attr.txt > PyBox_attr.c

// TODO: Check if centroid is calculated correctly for boxes that cross the Antimeridian!



void PyBox::dealloc(PyBox* self) 
{
    Py_TYPE(self)->tp_free(self);
}

PyObject* PyBox::create(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
    Py_ssize_t argCount = PySequence_Length(args);
    if (kwargs)
    {
        if (argCount)
        {
            PyErr_SetString(PyExc_TypeError, "Cannot mix positional and keyword arguments");
            return NULL;
        }
        Box box = Box::ofWorld();

        PyObject* key;
        PyObject* value;
        Py_ssize_t pos = 0;

        while (PyDict_Next(kwargs, &pos, &key, &value))
        {
            // References are borrowed, don't dec refcount

            Py_ssize_t len;
            const char* keyStr = PyUnicode_AsUTF8AndSize(key, &len);
            if (!keyStr) return NULL;
            Attribute* attr = PyBox_AttrHash::lookup(keyStr, len);
            if (!attr)
            {
                PyErr_Format(PyExc_TypeError, "Invalid argument: %s", keyStr);
                return NULL;
            }

            // Bits 8 & 9 contain the coordinate:
            //   0 = minX/left/west
            //   1 = minY/bottom/south
            //   2 = maxX/right/east
            //   3 = maxY/top/north
            // Bit 0 indicates whether to interpret coordinate 
            //   as GeoDesk Mercator (0) or WGS-84 (1)
            // Bit 1 indicates whether the coordinate value is both min and max

            int index = attr->index;
            int coordIndex = index >> 8;
            int32_t coordValue;
            if (index & 1)
            {
                // coordinate value represents lon/lat
                if (coordIndex & 1)
                {
                    PyCoordinate::ConversionResult yRes = PyCoordinate::yFromLat(value);
                    if (!yRes.success) return NULL;
                    coordValue =yRes.value;
                }
                else
                {
                    PyCoordinate::ConversionResult xRes = PyCoordinate::xFromLon(value);
                    if (!xRes.success) return NULL;
                    coordValue = xRes.value;
                }
            }
            else
            {
                // coordinate value is already in GeoDesk Mercator projection
                coordValue = PyLong_AsLong(value);
                if(coordValue == -1 && PyErr_Occurred()) return NULL;
                    // TODO: check if floor/ceil are needed (to make the most
                    // inclusive bbox), or if simple rounding is sufficient
            }
            box[coordIndex] = coordValue;
            // If needed, copy coordinate value also to max (for x,y,lon,lat)
            box[coordIndex + (index & 2)] = coordValue;
            // ignore overflow warning here, not an issue
        }
        return create(box);
    }

    switch (argCount)
    {
    case 0:
        return create(Box());

    case 1:
    {
        PyObject* arg = PyTuple_GET_ITEM(args, 0);
        PyTypeObject* type = Py_TYPE(arg);
        if (type == &TYPE) return create(((PyBox*)arg)->box);
        if (arg == Py_Ellipsis) return create(Box::ofWorld());
        geos::geom::Geometry* geom;
        if (Environment::get().getGeosGeometry(arg, (GEOSGeometry**) &geom))
        {
            return create(Box(geom->getEnvelopeInternal()));
        }
        PyErr_Format(PyExc_TypeError, "Invalid argument type: %s", type->tp_name);
        return NULL;
    }

    case 2:
    {
        Box box = Box::ofWorld();
        double v = getCoordValue(args, 0);
        if (v == -1.0 && PyErr_Occurred()) return NULL;
        box.setMinX((int32_t)v);
        box.setMaxX((int32_t)v);
        v = getCoordValue(args, 1);
        if (v == -1.0 && PyErr_Occurred()) return NULL;
        box.setMinY((int32_t)v);
        box.setMaxY((int32_t)v);
        return create(box);
    }
        
    case 4:
    {
        Box box = Box::ofWorld();
        double v = getCoordValue(args, 0);
        if (v == -1.0 && PyErr_Occurred()) return NULL;
        box.setMinX((int32_t)v);
        v = getCoordValue(args, 1);
        if (v == -1.0 && PyErr_Occurred()) return NULL;
        box.setMinY((int32_t)v);
        v = getCoordValue(args, 2);
        if (v == -1.0 && PyErr_Occurred()) return NULL;
        box.setMaxX((int32_t)v);
        v = getCoordValue(args, 3);
        if (v == -1.0 && PyErr_Occurred()) return NULL;
        box.setMaxY((int32_t)v);
        return create(box);
    }
    
    default:
        PyErr_SetString(PyExc_TypeError, "Must supply 2 or 4 coordinate values");
        return NULL;
    }
    
}

/*
    PyBox* self = Python::alloc<PyBox>(&TYPE);
    if (self) new(&self->box)Box();
    return (PyObject*)self;
*/

PyBox* PyBox::create(const Box& bbox)
{
    PyBox* self = Python::alloc<PyBox>(&TYPE);
    if (self) self->box = bbox;
    return self;
}

PyBox* PyBox::create(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    PyBox* self = Python::alloc<PyBox>(&TYPE);
    if (self) new(&self->box)Box(x1, y1, x2, y2);
    return self;
}

double getCoordValue(PyObject* seq, int index)
{
    PyObject* item = PySequence_GetItem(seq, index);
    // Returns a new reference 
    double val = PyFloat_AsDouble(item);
    Py_DECREF(item);
    return val;
}

// TODO: round coordinates *outwards*, so Box is always errs on the
// side of being more inclusive

int getCoordPair(PyObject* seq, int index, Coordinate& coord)
{
    double x = getCoordValue(seq, index);
    if (x == -1.0 && PyErr_Occurred()) return -1;
    double y = getCoordValue(seq, index + 1);
    coord = Coordinate((int32_t)x, (int32_t)y);
    return (y == -1.0 && PyErr_Occurred()) ? -1 : 0;
}



PyObject* PyBox::str(PyBox* self)
{
    return PyUnicode_FromFormat("[%d, %d] -> [%d, %d]", 
        self->box.minX(), self->box.minY(), 
        self->box.maxX(), self->box.maxY());
}

PyObject* PyBox::repr(PyBox* self)
{
    return PyUnicode_FromFormat("Box(%d, %d, %d, %d)",
        self->box.minX(), self->box.minY(),
        self->box.maxX(), self->box.maxY());
}


PyObject* PyBox::getattr(PyBox* self, PyObject* nameObj)
{
    Py_ssize_t len;
    const char* name = PyUnicode_AsUTF8AndSize(nameObj, &len);
    if (!name) return NULL;

    Attribute* attr = PyBox_AttrHash::lookup(name, len);
    if (attr)
    {
        // Bits 8 & 9 contain the coordinate:
        // 0 = minX/left/west
        // 1 = minY/bottom/south
        // 2 = maxX/right/east
        // 3 = maxY/top/north
        // Bit 0 indicates whether to return coordinate 
        //   as GeoDesk Mercator (0) or WGS-84 (1)
        int index = attr->index;
        int coordIndex = index >> 8;
        int32_t coordValue = self->box[coordIndex];
        if (index & 1)
        {
            double wgs84value = (coordIndex & 1) ? Mercator::latFromY(coordValue) :
                Mercator::lonFromX(coordValue);
            return PyFloat_FromDouble(PyCoordinate::precision7(wgs84value));
        }
        return PyLong_FromLong(coordValue);
    }

    if (strcmp(name, "buffer") == 0)
    {
        return PyFastMethod::create(self, (PyCFunctionWithKeywords)&buffer);
    }
    if (strcmp(name, "centroid") == 0)
    {
        Coordinate c = self->box.center();
        return PyCoordinate::create(c.x, c.y);
    }
    PyErr_SetString(PyExc_AttributeError, "Attribute not found");
    return nullptr;
}

Py_hash_t PyBox::hash(PyBox* self)
{
    return ((((Py_hash_t)self->box.minY()) << 32) | self->box.minX()) |
        ((((Py_hash_t)self->box.maxY()) << 32) | self->box.maxX());
}

int PyBox::contains(PyBox* self, PyObject* other)
{
    PyTypeObject* otherType = Py_TYPE(other);
    if (otherType == &PyCoordinate::TYPE)
    {
        PyCoordinate* coord = (PyCoordinate*)other;
        return self->box.contains(coord->x, coord->y);
    }
    if (otherType == &PyBox::TYPE)
    {
        PyBox* box = (PyBox*)other;
        return self->box.contains(box->box);
    }
    if (!PySequence_Check(other))
    {
        PyErr_Format(PyExc_TypeError, "Expected Coordinate, Box or <sequence> instead of %s", other->ob_type->tp_name);
        return -1;
    }

    // TODO: Could use PySequence_Fast
    // PyObject* seq = PySequence_Fast(other, "Expected Coordinate, Box or <sequence>");
    // be sure to check seq for null
    // be sure to decref seq after use

    Py_ssize_t count = PySequence_Length(other);
    if (count == 0) return 0;
    
    Coordinate c(0, 0);
    PyObject* item = PySequence_GetItem(other, 0);
    if (!item) return -1;
    if (PySequence_Check(item))
    {
        // We've got a sequence of sequences, e.g. [(x,y), (x,y)]
        Py_DECREF(item);
        for (int i = 0; i < count; i++)
        {
            item = PySequence_GetItem(other, i);
            if (!item) return -1;
            // TODO: If change to use fast protocol, check if 2+ items
            if (getCoordPair(item, 0, c) < 0)
            {
                Py_DECREF(item);
                return -1;
            }
            Py_DECREF(item);
            if (!self->box.contains(c)) return 0;
        }
        return 1;
    }

    // Sequence of coordinate pairs, e.g. [ x,y, x2, y2, ... ]
    Py_DECREF(item);
    if (count % 2)
    {
        PyErr_SetString(PyExc_TypeError, "Expected even number of coordinates (x,y,...)");
        return -1;
    }
    for (int i = 0; i < count; i += 2)
    {
        if (getCoordPair(other, i, c) < 0) return -1;
        if (!self->box.contains(c)) return 0;
    }
    return 1;
}

PyObject* PyBox::item(PyBox* self, Py_ssize_t index)
{
    if (index < 0 || index >= 4)
    {
        raiseIndexOutOfRange(index);
        return NULL;
    }
    return PyLong_FromLong(self->box[index]);   // TODO: Return float?
}

Py_ssize_t PyBox::len(PyObject*)
{
    return 4;
}

PyObject* PyBox::richcompare(PyBox* self, PyObject* other, int op)
{
    if (Py_TYPE(other) == &TYPE)
    {
        switch (op)
        {
        case Py_EQ:
            if (self->box == ((PyBox*)other)->box) Py_RETURN_TRUE;
            Py_RETURN_FALSE;
        case Py_NE:
            if (self->box != ((PyBox*)other)->box) Py_RETURN_TRUE;
            Py_RETURN_FALSE;
        default:
            Py_RETURN_NOTIMPLEMENTED;
        }
        
    }

    // TODO: Allow comparsion with tuples?

    Py_RETURN_NOTIMPLEMENTED;
}

int PyBox::isTrue(PyBox* self)
{
    return !self->box.isEmpty();
}


PyBox* PyBox::intersection(PyBox* self, PyObject* other)
{
    if (self->box.isEmpty()) return Python::newRef(self);
    if (Py_TYPE(other) == &TYPE)
    {
        PyBox* box = (PyBox*)other;
        if (box->box.isEmpty()) return Python::newRef(box);
        // TODO: Anti-Meridian
        return create(Box::simpleIntersection(self->box, box->box));
    }

    // TODO: accept sequence types?

    PyErr_Format(PyExc_TypeError, "Expected Box instead of %s", other->ob_type->tp_name);
    return NULL;
}

int32_t trimmedSubtract(int32_t x, int32_t y) 
{
    int32_t r = x - y;
    if (((x ^ y) & (x ^ r)) < 0) return std::numeric_limits<int32_t>::min();
    return r;
}

int32_t trimmedAdd(int32_t x, int32_t y) 
{
    int32_t r = x + y;
    if (((x ^ r) & (y ^ r)) < 0) return std::numeric_limits<int32_t>::max();
    return r;
}

/*
 void buffer(int32_t b) {
        minX -= b;
        maxX += b;
        if (b >= 0) {
            minY = trimmedSubtract(minY, b);
            maxY = trimmedAdd(maxY, b);
        } else {
            minY = trimmedAdd(minY, -b);
            maxY = trimmedSubtract(maxY, -b);
            if (maxY < minY) setNull();
            // TODO: check if width flipped
        }
    }
*/

int PyBox::doAdd(Box& box, PyObject* other)
{
    PyTypeObject* type = other->ob_type;
    if (type == &PyCoordinate::TYPE)
    {
        PyCoordinate* coord = (PyCoordinate*)other;
        box.expandToInclude(Coordinate(coord->x, coord->y));
        return 0;
    }
    PyErr_Format(PyExc_TypeError, "Not implemented yet for %s", other->ob_type->tp_name);
    return -1;
}

PyBox* PyBox::add(PyBox* self, PyObject* other)
{
    Box box = self->box;
    if(doAdd(box, other) < 0) return NULL;
    return create(box);
}

PyBox* PyBox::iadd(PyBox* self, PyObject* other)
{
    if (doAdd(self->box, other) < 0) return NULL;
    return Python::newRef(self);
}

PyBox* PyBox::buffer(PyBox* self, PyObject* args, PyObject* kwargs)
{
    double distance;
    if (kwargs)
    {
        PyObject* key;
        PyObject* value;
        Py_ssize_t pos = 0;
        if (PyDict_Next(kwargs, &pos, &key, &value))
        {
            Py_ssize_t len;
            const char* keyStr = PyUnicode_AsUTF8AndSize(key, &len);
            int units = LengthUnit::unitFromString(std::string_view(keyStr, len));
            if (units < 0)
            {
                Python::badKeyword(keyStr);
                return NULL;
            }
            distance = PyFloat_AsDouble(value);
            if (distance == -1.0 && PyErr_Occurred()) return NULL;
            distance = Mercator::unitsFromMeters(
                LengthUnit::toMeters(distance, units),
                (self->box.minY() + self->box.maxY()) / 2);
            if (PyDict_Next(kwargs, &pos, &key, &value))
            {
                // There should only be 1 keyword argument
                PyErr_SetString(PyExc_TypeError, "Invalid keyword arguments");
                return NULL;
            }
            goto valid_distance;
        }
    }

    // fall through

    {
        Py_ssize_t argCount = PySequence_Length(args);
        if (argCount != 1)
        {
            PyErr_SetString(PyExc_TypeError, "Expected <distance>");
            return NULL;
        }
        PyObject* arg = PyTuple_GET_ITEM(args, 0);
        distance = PyFloat_AsDouble(arg);
        if (distance == -1.0 && PyErr_Occurred()) return NULL;
    }

valid_distance:
    self->box.buffer(static_cast<int32_t>(round(distance)));
    return Python::newRef(self);
}


PySequenceMethods PyBox::SEQUENCE_METHODS =
{
    .sq_length = &len,
    .sq_item = (ssizeargfunc)item,
    .sq_contains = (objobjproc)&contains,
};

PyNumberMethods PyBox::NUMBER_METHODS =
{
    .nb_add = (binaryfunc)add,
    .nb_bool = (inquiry)isTrue,
    .nb_and = (binaryfunc)intersection,
    .nb_inplace_add = (binaryfunc)iadd,
};

PyTypeObject PyBox::TYPE =
{
    .tp_name = "geodesk.Box",
    .tp_basicsize = sizeof(PyBox),
    .tp_dealloc = (destructor)PyBox::dealloc,
    .tp_repr = (reprfunc)PyBox::repr,
    .tp_as_number = &NUMBER_METHODS,
    .tp_as_sequence = &SEQUENCE_METHODS,
    // tp_as_mapping 
    .tp_hash = (hashfunc)hash,
    // tp_call 
    .tp_str = (reprfunc)PyBox::str,
    .tp_getattro = (getattrofunc)PyBox::getattr,
    .tp_flags = Py_TPFLAGS_DEFAULT, 
    .tp_doc = "Box objects", 
    .tp_richcompare = (richcmpfunc)richcompare,
    // .tp_init = (initproc)PyBox::init,
    .tp_new = PyBox::create
};

