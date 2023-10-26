// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyFeatures.h"
#include "filter/Filter.h"
#include "python/feature/PyFeature.h"
#include "python/query/PyQuery.h"

// TODO: Doesn't work, we can't target anonymous nodes this way
// (They cannot be referenced via a pointer because they don't exist)

PyObject* PyFeatures::Parents::iterFeatures(PyFeatures* features)
{
    FeatureTypes acceptedTypes = features->acceptedTypes;
    FeatureRef feature = features->relatedFeature;
    if ((acceptedTypes & FeatureTypes::WAYS) == 0)
    {
        // only parent relations accepted

        return PyParentRelationIterator::create(features, feature.relationTableFast());
    }
    else
    {
        assert(feature.isNode());
        if ((acceptedTypes & FeatureTypes::RELATIONS) == 0)
        {
            // only parent ways
            Py_RETURN_NONE;  // TODO
        }
        else
        {
            // both parent ways & parent relations
            Py_RETURN_NONE;  // TODO
        }
    }
}

int PyFeatures::Parents::isEmpty(PyFeatures* features)
{
    // TODO: can shortcut if non-filtered
    return PyFeatures::isEmpty(features);
}

SelectionType PyFeatures::Parents::SUBTYPE =
{
    iterFeatures,
    countFeatures,
    isEmpty
};


PyObject* PyParentRelationIterator::create(PyFeatures* features, pointer pRelTable)
{
    PyParentRelationIterator* self = (PyParentRelationIterator*)TYPE.tp_alloc(&TYPE, 0);
    if (self)
    {
        self->target = Python::newRef(features);
        // By holding a reference to the PyFeatures, we ensure that the
        // FeatureStore, the matcher and the filter stay alive as long as
        // the PyMemberIterator is alive
        new(&self->iter)ParentRelationIterator(features->store, pRelTable,
            features->matcher, features->filter);
    }
    return (PyObject*)self;
}

void PyParentRelationIterator::dealloc(PyParentRelationIterator* self)
{
    Py_DECREF(self->target);
    self->iter.~ParentRelationIterator();
    Py_TYPE(self)->tp_free(self);
}

PyObject* PyParentRelationIterator::next(PyParentRelationIterator* self)
{
    RelationRef rel = self->iter.next();
    if (rel.isNull()) return NULL;
    return PyFeature::create(self->iter.store(), rel, Py_None);
}

PyTypeObject PyParentRelationIterator::TYPE =
{
    .tp_name = "geodesk.ParentRelationIterator",
    .tp_basicsize = sizeof(PyParentRelationIterator),
    .tp_dealloc = (destructor)dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT, // | Py_TPFLAGS_DISALLOW_INSTANTIATION,
    .tp_iter = PyObject_SelfIter,
    .tp_iternext = (iternextfunc)next,
};


