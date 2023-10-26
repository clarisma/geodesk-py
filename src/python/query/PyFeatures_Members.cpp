// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyFeatures.h"
#include "python/feature/PyFeature.h"



PyObject* PyFeatures::Members::iterFeatures(PyFeatures* features)
{
    return PyMemberIterator::create(features);
}

int PyFeatures::Members::isEmpty(PyFeatures* features)
{
    // TODO: can shortcut if non-filtered
    return PyFeatures::isEmpty(features);
}

SelectionType PyFeatures::Members::SUBTYPE =
{
    iterFeatures,
    countFeatures,
    isEmpty
};


PyObject* PyMemberIterator::create(PyFeatures* features)
{
    pointer pBody = features->relatedFeature.bodyptr();
    PyMemberIterator* self = (PyMemberIterator*)TYPE.tp_alloc(&TYPE, 0);
    if (self)
    {
        self->target = Python::newRef(features);
        // By holding a reference to the PyFeatures, we ensure that the
        // FeatureStore, the matcher and the filter stay alive as long as
        // the PyMemberIterator is alive
        new(&self->iter)MemberIterator(features->store, pBody,
            features->acceptedTypes, features->matcher, features->filter);
    }
    return (PyObject*)self;
}

PyObject* PyMemberIterator::create(PyFeature* rel)
{
    pointer pBody = rel->feature.bodyptr();
    PyMemberIterator* self = (PyMemberIterator*)TYPE.tp_alloc(&TYPE, 0);
    if (self)
    {
        FeatureStore* store = rel->store;
        self->target = Python::newRef(rel);
        // By holding a reference to the relation, we ensure that the
        // FeatureStore (and it's all-matcher) stay alive as long as
        // the PyMemberIterator is alive
        new(&self->iter)MemberIterator(store, pBody, FeatureTypes::ALL,
            store->borrowAllMatcher(), NULL);
    }
    return (PyObject*)self;
}

void PyMemberIterator::dealloc(PyMemberIterator* self)
{
    Py_DECREF(self->target);
    self->iter.~MemberIterator();
    Py_TYPE(self)->tp_free(self);
}

PyObject* PyMemberIterator::next(PyMemberIterator* self)
{
    FeatureRef feature = self->iter.next();
    if (feature.isNull()) return NULL;
    return PyFeature::create(self->iter.store(), feature, self->iter.borrowCurrentRole());
}

PyTypeObject PyMemberIterator::TYPE =
{
    .tp_name = "geodesk.MemberIterator",
    .tp_basicsize = sizeof(PyMemberIterator),
    .tp_dealloc = (destructor)dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT, // | Py_TPFLAGS_DISALLOW_INSTANTIATION,
    .tp_iter = PyObject_SelfIter,
    .tp_iternext = (iternextfunc)next,
};


