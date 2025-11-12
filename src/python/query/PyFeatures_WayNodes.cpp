// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyFeatures.h"
#include <clarisma/util/log.h>
#include "python/feature/PyFeature.h"

PyFeatures* PyFeatures::WayNodes::createRelated(PyFeatures* base, WayPtr way)
{
    // TODO: verify feature
    return PyFeatures::createRelated(base, &PyFeatures::WayNodes::SUBTYPE, 
        way, FeatureTypes::NODES);
}

// TODO: if matcher is present but way does not have feature nodes,
// result is an empty set
// --> need to override withQuery()

PyObject* PyFeatures::WayNodes::iterFeatures(PyFeatures* features)
{
    return PyWayNodeIterator::create(features);
}

PyObject* PyFeatures::WayNodes::countFeatures(PyFeatures* self)
{
    // TODO: fix and re-enable
    /*
    if (self->acceptsAny())
    {
        WayRef way(self->relatedFeature);
        return way.nodeCount();
    }
    */
    return PyFeatures::countFeatures(self);
}

int PyFeatures::WayNodes::isEmpty(PyFeatures* self)
{
    // If a way's set of nodes is unconstrained, it cannot be empty,
    // since a way by definition always has at least 2 nodes.
    // (This is true even for ways that are placeholders)
    if (self->acceptsAny())
    {
        return 0;
    }
    return PyFeatures::isEmpty(self);
}

SelectionType PyFeatures::WayNodes::SUBTYPE =
{
    iterFeatures,
    countFeatures,
    isEmpty,
    containsFeature,
    getTiles
};

// TODO: containsFeature should check for type ndoe and way-node flag to rule out given feature

PyObject* PyWayNodeIterator::create(PyFeatures* features)
{
    WayPtr way(features->relatedFeature);
    int flags = way.flags();
    DataPtr pBody = way.bodyptr();
    PyWayNodeIterator* self = (PyWayNodeIterator*)TYPE.tp_alloc(&TYPE, 0);
    if (self)
    {
        self->featureNodesOnly = features->flags & SelectionFlags::USES_MATCHER;
        self->target = Python::newRef(features);
        new(&self->nodeCursor)WayNodeCursor(way, features->store->hasWaynodeIds());
        new(&self->featureIter)FeatureNodeIterator(features->store, way,
            features->matcher, features->filter);
        self->nextNode = self->featureIter.next();
    }
    return (PyObject*)self;
}

// TODO: consolidate this code?
// May not need it if we stop treating features as iterable
PyObject* PyWayNodeIterator::create(PyFeature* wayObj)
{
    WayPtr way(wayObj->feature);
    bool wayNodeIds = wayObj->store->hasWaynodeIds();
    // LOG("Iterating way/%ld", way.id());
    int flags = way.flags();
    DataPtr pBody = way.bodyptr();
    PyWayNodeIterator* self = (PyWayNodeIterator*)TYPE.tp_alloc(&TYPE, 0);
    if (self)
    {
        self->target = Python::newRef(wayObj);
        self->featureNodesOnly = false;
        new(&self->nodeCursor)WayNodeCursor(way, wayNodeIds);
        new(&self->featureIter)FeatureNodeIterator(wayObj->store, way);
        self->nextNode = self->featureIter.next();
    }
    return (PyObject*)self;
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
