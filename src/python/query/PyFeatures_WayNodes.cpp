// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyFeatures.h"
#include <clarisma/util/log.h>
#include "python/feature/PyFeature.h"

using namespace clarisma;


PyObject* PyWayNodeIterator::create(PyFeatures* features)
{
    WayPtr way(features->relatedFeature());
    PyWayNodeIterator* self = (PyWayNodeIterator*)TYPE.tp_alloc(&TYPE, 0);
    if (self)
    {
        self->featureNodesOnly = features->usesMatcher();
        self->target = Python::newRef(features);
        new(&self->nodeCursor)WayNodeCursor(way, features->store()->hasWaynodeIds());
        new(&self->featureIter)FeatureNodeIterator(features->store(), way,
            features->matcher(), features->filter());
        self->nextNode = self->featureIter.next();
    }
    return self;
}

// TODO: consolidate this code?
// May not need it if we stop treating features as iterable
PyObject* PyWayNodeIterator::create(PyFeature* wayObj)
{
    WayPtr way(wayObj->feature);
    bool wayNodeIds = wayObj->store->hasWaynodeIds();
    PyWayNodeIterator* self = (PyWayNodeIterator*)TYPE.tp_alloc(&TYPE, 0);
    if (self)
    {
        self->target = Python::newRef(wayObj);
        self->featureNodesOnly = false;
        new(&self->nodeCursor)WayNodeCursor(way, wayNodeIds);
        new(&self->featureIter)FeatureNodeIterator(wayObj->store, way);
        self->nextNode = self->featureIter.next();
    }
    return self;
}

void PyWayNodeIterator::dealloc(PyWayNodeIterator* self)
{
    Py_DECREF(self->target);
    Py_TYPE(self)->tp_free(self);
}

PyObject* PyWayNodeIterator::next(PyWayNodeIterator* self)
{
    // TODO: improve this control flow

    if (self->featureNodesOnly)
    {
        NodePtr nextNode = self->nextNode;
        if (nextNode.isNull()) return NULL;
        self->nextNode = self->featureIter.next();
        return PyFeature::create(self->featureIter.store(), nextNode, Py_None);
    }
    Coordinate c = self->nodeCursor.xy();
    if (c.isNull()) return NULL;
    uint64_t nodeId = self->nodeCursor.id();
    (void)self->nodeCursor.next();
    NodePtr nextNode = self->nextNode;
    FeatureStore* store = self->featureIter.store();
    if (!nextNode.isNull() && nextNode.xy() == c)
    {
        self->nextNode = self->featureIter.next();
        return PyFeature::create(store, nextNode, Py_None);
    }
    return PyAnonymousNode::create(store, nodeId, c.x, c.y);
}

PyTypeObject PyWayNodeIterator::TYPE =
{
    .tp_name = "geodesk.WayNodeIterator",
    .tp_basicsize = sizeof(PyWayNodeIterator),
    .tp_dealloc = (destructor)dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT, // | Py_TPFLAGS_DISALLOW_INSTANTIATION,
    .tp_iter = PyObject_SelfIter,
    .tp_iternext = (iternextfunc)next,
};
