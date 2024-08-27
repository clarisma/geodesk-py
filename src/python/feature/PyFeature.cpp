// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyFeature.h"
#include <common/math/Math.h>
#include <common/util/log.h>
#include "python/Environment.h"
#include "python/format/PyMap.h"
#include "python/geom/PyBox.h"
#include "python/geom/PyCoordinate.h"
#include "python/query/PyFeatures.h"
#include "python/util/PyFastMethod.h"
#include "python/util/util.h"
#include "PyTags.h"
#include "geom/Mercator.h"

#include "PyFeature_lookup.cxx"


// TODO: adding feature IDs to a set is 3x faster than adding the actual feature to a set!
//  Try storing ID (and type bits) in PyFeature object itself

PyFeature* PyFeature::create(FeatureStore* store, FeatureRef feature, PyObject* role)
{
    PyFeature* self = (PyFeature*)TYPE.tp_alloc(&TYPE, 0);
    if (self)
    {
        store->addref();
        self->store = store;
        self->feature = feature;
        Py_INCREF(role);
        self->roleString = role;
    }
    return self;
}

void PyFeature::dealloc(PyFeature* self)
{
    Py_DECREF(self->roleString);
    self->store->release();
    Py_TYPE(self)->tp_free(self);
}

PyObject* PyFeature::iter(PyFeature* self)
{
    if (self->feature.isWay())
    {
        return PyWayNodeIterator::create(self);
    }
    if (self->feature.isRelation())
    {
        return PyMemberIterator::create(self);
    }
    // TODO: empty iterator
    // TODO: fix this !!!!!
    Py_RETURN_NONE;
}

PyObject* PyFeature::richcompare(PyFeature* self, PyObject* other, int op)
{
    if (Py_TYPE(self) == Py_TYPE(other)) 
    {
        PyFeature* otherFeature = (PyFeature*)other;
        switch (op) 
        {
        case Py_EQ:
            if (self->feature.id() == otherFeature->feature.id() &&
                self->store == otherFeature->store)
            {
                Py_RETURN_TRUE;
            }
            else 
            {
                Py_RETURN_FALSE;
            }
        case Py_NE:
            if (self->feature.id() != otherFeature->feature.id() ||
                self->store != otherFeature->store)
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

Py_hash_t PyFeature::hash(PyFeature* self)
{
    return self->feature.hash();
}

PyObject* PyFeature::str(PyFeature* self)
{
    return PyUnicode_FromFormat("%s/%llu",
        self->feature.typeName(), self->feature.id());
}

PyObject* PyFeature::return_zero(PyFeature* self)
{
    return PyLong_FromLong(0);
}

PyObject* PyFeature::return_true(PyFeature* self)
{
    Py_RETURN_TRUE;
}

PyObject* PyFeature::return_false(PyFeature* self)
{
    Py_RETURN_FALSE;
}

PyObject* PyFeature::return_none(PyFeature* self)
{
    Py_RETURN_NONE;
}

PyObject* PyFeature::return_blank(PyFeature* self)
{
    return Environment::get().getString(Environment::Strings::BLANK);
}

PyObject* PyFeature::return_empty(PyFeature* self)
{
    // return (PyObject*)Environment::get().getEmptyFeatures();
    return self->store->getEmptyFeatures();
}



PyObject* PyFeature::bounds(PyFeature* self)
{
    // TODO: This assumes layout of Box matches feature store bboxes
    // for now, this is true: minX, minY, maxX, maxY
    const Box* pBox = reinterpret_cast<const Box*>(
        self->feature.asBytePointer() - 16);
    return PyBox::create(*pBox);
}

PyObject* PyFeature::id(PyFeature* self)
{
    return PyLong_FromLongLong(self->feature.id());
}

PyObject* PyFeature::is_area(PyFeature* self)
{
    return PyBool_FromLong(self->feature.isArea());
}

PyObject* PyFeature::lat(PyFeature* self)
{
    pointer p = self->feature.ptr();
    return PyCoordinate::niceLatFromY(Math::avg(p.getInt(-12), p.getInt(-4)));
}

PyObject* PyFeature::lon(PyFeature* self)
{
    pointer p = self->feature.ptr();
    return PyCoordinate::niceLonFromX(Math::avg(p.getInt(-16), p.getInt(-8)));
}

PyObject* PyFeature::map(PyFeature* self)
{
    return PyMap::create(self);
}

PyObject* PyFeature::parents(PyFeature* self)
{
    // Default implementation, valid only for ways and relations
    if ((self->feature.flags() & FeatureFlags::RELATION_MEMBER) == 0)
    {
        return return_empty(self);
    }
    // TODO: not needed here
    // For ways and relations, the reltable pointer is placed 
    // just ahead of the body anchor
    return PyFeatures::create(&PyFeatures::Parents::SUBTYPE,
        self->store, self->feature, FeatureTypes::RELATIONS);
}

PyObject* PyFeature::num_method(PyFeature* self)
{
    return PyFastMethod::create(self, (PyCFunctionWithKeywords)&numTagValue);
}

PyObject* PyFeature::osm_type(PyFeature* self)
{

   // return PyUnicode_FromString(self->feature.typeName());
    int typeCode = self->feature.typeCode();
    return Environment::get().getString(Environment::Strings::NODE + typeCode);
}



PyObject* PyFeature::role(PyFeature* self)
{
    PyObject* role = self->roleString;
    Py_INCREF(role);
    return role;
}


PyObject* PyFeature::str_method(PyFeature* self)
{
    return PyFastMethod::create(self, (PyCFunctionWithKeywords)&strTagValue);
}

PyObject* PyFeature::tags(PyFeature* self)
{
    return PyTags::create(self->store, self->feature.tags());
}


PyObject* PyFeature::x(PyFeature* self)
{
    pointer p = self->feature.ptr();
    return PyLong_FromLong(Math::avg(p.getInt(-16), p.getInt(-8)));
}

PyObject* PyFeature::y(PyFeature* self)
{
    pointer p = self->feature.ptr();
    return PyLong_FromLong(Math::avg(p.getInt(-12), p.getInt(-4)));
}


PyObject* PyFeature::strTagValue(PyFeature* self, PyObject* args, PyObject* kwargs)
{
    PyObject* keyObj = Python::checkSingleArg(args, kwargs, &PyUnicode_Type);
    const TagsRef& tags = self->feature.tags();
    StringTable& strings = self->store->strings();
    int64_t value = tags.getKeyValue(keyObj, strings);
    return tags.valueAsString(value, strings);
}


PyObject* PyFeature::numTagValue(PyFeature* self, PyObject* args, PyObject* kwargs)
{
    PyObject* keyObj = Python::checkSingleArg(args, kwargs, &PyUnicode_Type);
    const TagsRef& tags = self->feature.tags();
    StringTable& strings = self->store->strings();
    int64_t value = tags.getKeyValue(keyObj, strings);
    return tags.valueAsNumber(value, strings);
}



// TODO: getattr must handle class-level lookups as well, self could be a PyTypeObject
// or a PyFeature

const AttrFunctionPtr* PyFeature::SUBTYPE_FEATURE_METHODS[] =
{
    Node::FEATURE_METHODS,
    Way::FEATURE_METHODS,
    Relation::FEATURE_METHODS,
    Way::FEATURE_METHODS,        // TODO: type 3 is invalid
};


PyObject* PyFeature::getattr(PyFeature* self, PyObject* nameObj)
{
    int typeCode = self->feature.typeCode();
    return PyFeature::getattr0(self, nameObj, SUBTYPE_FEATURE_METHODS[typeCode]);
}

PyObject* PyFeature::getattr0(PyFeature* self, PyObject* nameObj, const AttrFunctionPtr* const table)
{
    Py_ssize_t len;
    const char* name = PyUnicode_AsUTF8AndSize(nameObj, &len);
    if (!name) return NULL;

    Attribute* attr = PyFeature_AttrHash::lookup(name, len);
    if (attr)
    {
         return (*table[attr->index])(self);
    }
    const TagsRef& tags = self->feature.tags();
    return tags.getValue(nameObj, self->store->strings());
}

PyObject* PyFeature::getBuiltinAttr(PyFeature* self, PyObject* nameObj, const AttrFunctionPtr* const table)
{
    Py_ssize_t len;
    const char* name = PyUnicode_AsUTF8AndSize(nameObj, &len);
    if (!name) return NULL;

    Attribute* attr = PyFeature_AttrHash::lookup(name, len);
    if (attr) return (*table[attr->index])(self);
    Py_RETURN_NONE;
}

int PyFeature::setattr(PyFeature* self, PyObject* nameObj, PyObject* value)
{
    /*
    Py_ssize_t len;
    const char* name = PyUnicode_AsUTF8AndSize(nameObj, &len);
    if (!name) return -1;

    Attribute* attr = PyFeature_lookupAttr(name, len);
    if (attr)
    {
        // TODO: Allow "role" to be modified?
        PyErr_Format(PyExc_AttributeError, "Attribute '%s' is read-only", name);
        return -1;
    }
    return PyObject_GenericSetAttr((PyObject*)self, nameObj, value);
    */
    PyErr_Format(PyExc_AttributeError, "Attributes of '%s' are read-only", 
        Py_TYPE(self)->tp_name);
    return -1;
}

PyObject* PyFeature::subscript(PyFeature* self, PyObject* keyObj)
{
    if (!PyUnicode_Check(keyObj)) 
    {
        PyErr_SetString(PyExc_TypeError, "Key must be a string");
        return NULL;
    }
    const TagsRef& tags = self->feature.tags();
    return tags.getValue(keyObj, self->store->strings());
}

PyMappingMethods PyFeature::MAPPING_METHODS =
{
    nullptr,         // mp_length
    reinterpret_cast<binaryfunc>(PyFeature::subscript), // mp_subscript
    nullptr          // mp_ass_subscript
};

PyTypeObject PyFeature::TYPE =
{
    .tp_name = "geodesk.Feature",
    .tp_basicsize = sizeof(PyFeature),
    .tp_dealloc = (destructor)PyFeature::dealloc,
    .tp_repr = (reprfunc)PyFeature::str,
    .tp_as_mapping = &MAPPING_METHODS,
    .tp_hash = (hashfunc)hash,
    .tp_str = (reprfunc)PyFeature::str,
    .tp_getattro = (getattrofunc)PyFeature::getattr,
    .tp_setattro = (setattrofunc)PyFeature::setattr, 
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // | Py_TPFLAGS_DISALLOW_INSTANTIATION,
    .tp_doc = "Feature objects",
    .tp_richcompare = (richcmpfunc)richcompare,          
    .tp_iter = (getiterfunc)iter,
};

