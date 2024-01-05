// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyFeature.h"
#include "python/Environment.h"
#include "python/format/PyFormatter.h"
#include "python/geom/PyBox.h"
#include "python/geom/PyCoordinate.h"
#include "feature/GeometryBuilder.h"
#include "geom/Mercator.h"

PyObject* PyAnonymousNode::create(FeatureStore* store, int32_t x, int32_t y)
{
    PyAnonymousNode* self = (PyAnonymousNode*)
        PyAnonymousNode::TYPE.tp_alloc(&PyAnonymousNode::TYPE, 0);

    if (self)
    {
        store->addref();
        self->store = store;
        self->x_ = x;
        self->y_ = y;
    }
    return (PyObject*)self;
}

void PyAnonymousNode::dealloc(PyAnonymousNode* self)
{
    self->store->release();
    Py_TYPE(self)->tp_free(self);
}

PyObject* PyAnonymousNode::str(PyAnonymousNode* self)
{
    return PyUnicode_FromFormat("node@%d,%d", self->x_, self->y_);
}

PyObject* PyAnonymousNode::bounds(PyAnonymousNode* self)
{
    return PyBox::create(self->x_, self->y_, self->x_, self->y_);
}

PyObject* PyAnonymousNode::centroid(PyAnonymousNode* self)
{
    return PyCoordinate::create(self->x_, self->y_);
}


PyObject* PyAnonymousNode::lat(PyAnonymousNode* self)
{
    return PyFloat_FromDouble(Mercator::latFromY(self->y_));
}

PyObject* PyAnonymousNode::lon(PyAnonymousNode* self)
{
    return PyFloat_FromDouble(Mercator::lonFromX(self->x_));
}

PyObject* PyAnonymousNode::osm_type(PyFeature* self)
{
    return Environment::get().getString(Environment::Strings::NODE);
}

PyObject* PyAnonymousNode::parents(PyAnonymousNode* self)
{
    // TODO
    return PyFeature::return_empty(NULL);
}


PyObject* PyAnonymousNode::shape(PyAnonymousNode* self)
{
    Environment& env = Environment::get();
    GEOSContextHandle_t geosContext = env.getGeosContext();
    GEOSGeometry* geom = GeometryBuilder::buildPointGeometry(self->x_, self->y_, geosContext);
    return env.buildShapelyGeometry(geom);
}

PyObject* PyAnonymousNode::tags(PyAnonymousNode* self)
{
    return self->store->emptyTags();
}

PyObject* PyAnonymousNode::x(PyAnonymousNode* self)
{
    return PyLong_FromLong(self->x_);
}

PyObject* PyAnonymousNode::y(PyAnonymousNode* self)
{
    return PyLong_FromLong(self->y_);
}

PyObject* PyAnonymousNode::getattr(PyAnonymousNode* self, PyObject* nameObj)
{
    return PyFeature::getBuiltinAttr((PyFeature*)self, nameObj, PyAnonymousNode::FEATURE_METHODS);
    // The cast here is ok, even though PyAnonymousNode does not derive from PyFeature
    // getattr0 simply passes the pointer to the resolved method (which takes a pointer
    // to a PyAnonymousNode)
}


Py_hash_t PyAnonymousNode::hash(PyAnonymousNode* self)
{
    return (int64_t)self->x_ | ((int64_t)self->y_ << 32);
}

PyObject* PyAnonymousNode::richcompare(PyAnonymousNode* self, PyObject* other, int op)
{
    if (Py_TYPE(other) == &TYPE)
    {
        PyAnonymousNode* otherNode = (PyAnonymousNode*)other;
        switch (op)
        {
        case Py_EQ:
            if (self->x_ == otherNode->x_ &&
                self->y_ == otherNode->y_ &&
                self->store == otherNode->store)
            {
                Py_RETURN_TRUE;
            }
            else
            {
                Py_RETURN_FALSE;
            }
        case Py_NE:
            if (self->x_ != otherNode->x_ ||
                self->y_ != otherNode->y_ ||
                self->store != otherNode->store)
            {
                Py_RETURN_TRUE;
            }
            else
            {
                Py_RETURN_FALSE;
            }
        default:
            Py_RETURN_NOTIMPLEMENTED;
        }
    }
    else
    {
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
}

PyObject* PyAnonymousNode::subscript(PyAnonymousNode* self, PyObject* keyObj)
{
    Py_RETURN_NONE;
}


AttrFunctionPtr const PyAnonymousNode::FEATURE_METHODS[] =
{
    PyFeature::return_zero,        // area
    (AttrFunctionPtr)bounds,       // bounds
    (AttrFunctionPtr)centroid,     // centroid
    (AttrFunctionPtr)PyFormatter::geojson,          // geojson -- TODO
    PyFeature::return_zero,        // id
    PyFeature::return_false,       // is_area
    PyFeature::return_true,        // is_node
    PyFeature::return_false,       // is_placeholder
    PyFeature::return_false,       // is_relation
    PyFeature::return_false,       // is_way
    (AttrFunctionPtr)lat,          // lat
    PyFeature::return_zero,        // length
    (AttrFunctionPtr)lon,          // lon
    PyFeature::map,                // map -- TODO
    PyFeature::return_empty,       // members
    PyFeature::return_empty,       // nodes
    PyFeature::return_zero,        // num
    (AttrFunctionPtr)osm_type,     // osm_type
    (AttrFunctionPtr)parents,      // parents
    PyFeature::return_none,        // role
    (AttrFunctionPtr)shape,        // shape 
    PyFeature::return_blank,       // str
    (AttrFunctionPtr)tags,         // tags
    (AttrFunctionPtr)PyFormatter::wkt,              // wkt -- TODO
    (AttrFunctionPtr)x,            // x
    (AttrFunctionPtr)y,            // y
    PyFeature::return_zero,        // fast_area
};


PyMappingMethods PyAnonymousNode::MAPPING_METHODS =
{
    nullptr,         // mp_length
    (binaryfunc)subscript, // mp_subscript
    nullptr          // mp_ass_subscript
};


PyTypeObject PyAnonymousNode::TYPE =
{
    .tp_name = "geodesk.AnonymousNode",
    .tp_basicsize = sizeof(PyAnonymousNode),
    .tp_dealloc = (destructor)dealloc,
    .tp_repr = (reprfunc)str,
    .tp_as_mapping = &MAPPING_METHODS,
    .tp_hash = (hashfunc)hash,
    .tp_str = (reprfunc)str,
    .tp_getattro = (getattrofunc)getattr,
    .tp_setattro = (setattrofunc)PyFeature::setattr,    // correct
    .tp_flags = Py_TPFLAGS_DEFAULT, // | Py_TPFLAGS_DISALLOW_INSTANTIATION,
    .tp_doc = "AnonymousNode objects",
    .tp_richcompare = (richcmpfunc)richcompare,
    // TODO: tp_iter: this is wrong!!!!
    .tp_iter = (getiterfunc)PyFeature::return_none,
    .tp_base = &PyFeature::TYPE,
};

