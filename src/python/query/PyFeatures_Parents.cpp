// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyFeatures.h"
#include "filter/Filter.h"
#include "python/feature/PyFeature.h"
#include "python/query/PyQuery.h"

// ... can have ... as parents:
// feature nodes:   
//   - ways
//   - relations
//   - ways and relations
//   - none
// anonymous nodes: 
//   - ways
// ways:            
//   - relations
//   - none
// relations:       
//   - relations
//   -none
 


PyFeatures* PyFeatures::Parents::create(PyAnonymousNode* relatedNode)
{
    PyFeatures* self = (PyFeatures*)TYPE.tp_alloc(&TYPE, 0);
    if (self)
    {
        self->selectionType = &SUBTYPE;
        FeatureStore* store = relatedNode->store;
        store->addref();
        self->store = store;
        self->flags = SelectionFlags::USES_BOUNDS;
            // Instead of relatedFeature (which must be a PyFeature),
            // we use the bounding box (since an anonymous node is
            // uniquely described by its location)
            // The bounding box just consists of a single pixel
        self->bounds = Box(Coordinate(relatedNode->x_, relatedNode->y_));
        self->acceptedTypes = FeatureTypes::WAYS;
            // For anonymous nodes, we need to iterate all ways,
            // not just the ways with the way-node flag set
            // (because that flag only indicates that the way
            // has one or more feature nodes)
        self->matcher = store->getAllMatcher();
        self->filter = nullptr;
    }
    return self;
}


PyObject* PyFeatures::Parents::iterFeatures(PyFeatures* features)
{
    if (features->flags & SelectionFlags::USES_BOUNDS)
    {
        // for anonymous nodes, we use bounds (a single-pixel bbox
        // that describes the nodes location) rather than relatedFeature
        // (because they don't exist as a feature)
        // We only iterate their ways, becuase by definition they
        // can never be relation members
        return PyNodeParentIterator::create(features, features->bounds.bottomLeft());
    }
    else
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
            NodeRef node(feature);
            return (PyObject*)PyNodeParentIterator::create(features, node, 
                (acceptedTypes & FeatureTypes::RELATIONS) == 0 ?
                PyNodeParentIterator::IterationStatus::WAYS :
                PyNodeParentIterator::IterationStatus::RELATIONS);
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
    isEmpty,
    containsFeature,
    getTiles
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



bool FeatureNodeFilter::accept(FeatureStore* store, FeatureRef feature, FastFilterHint fast) const 
{
    // TODO: Ignore missing tiles; by definition, we cannot find the
    // candidate node in a missing tile
    // LOG("Checking %s...", feature.toString().c_str());
    assert(feature.isWay());
    WayRef way(feature);
    FeatureNodeIterator iter(store);
    iter.start(way.bodyptr(), way.flags(), store->borrowAllMatcher(), nullptr);
    for (;;)
    {
        NodeRef node = iter.next();
        if (node.isNull()) return false;
        if (node.asBytePointer() == node_.asBytePointer()) break;
    }
    return !secondaryFilter_ || secondaryFilter_->accept(store, feature, fast);
}


bool WayNodeFilter::accept(FeatureStore* store, FeatureRef feature, FastFilterHint fast) const 
{
    assert(feature.isWay());
    WayRef way(feature);
    // LOG("Checking way/%llu", way.id());
    WayCoordinateIterator iter;
    iter.start(way, 0);
    for (;;)
    {
        Coordinate c = iter.next();
        if (c.isNull())
        {
            // LOG("  Not accepted.");
            return false;
        }
        if (c == coord_) break;
    }
    // LOG("  Accepted (prior to secondary filter)!");
    return !secondaryFilter_ || secondaryFilter_->accept(store, feature, fast);
}


PyObject* PyNodeParentIterator::create(PyFeatures* features, Coordinate wayNodeXY)
{
    PyNodeParentIterator* self = (PyNodeParentIterator*)TYPE.tp_alloc(&TYPE, 0);
    if (self)
    {
        self->target = Python::newRef(features);
        // By holding a reference to the PyFeatures, we ensure that the
        // FeatureStore, the matcher and the filter stay alive as long as
        // the PyNodeParentIterator is alive
        self->status = IterationStatus::WAYS;
        // We start directly with ways, since anonymous nodes cannot be 
        // relation members
        const Filter* secondaryFilter = features->filter;
        new(&self->wayNodeFilter)WayNodeFilter(wayNodeXY, secondaryFilter);
        // We don't change refcount of the secondary filter, since we can
        // be assured that is alive as long as this iterator is alive (because
        // `target` holds a strong reference to Features)
        self->wayQuery = PyQuery::create(features, Box(wayNodeXY),
            features->acceptedTypes, features->matcher, &self->wayNodeFilter);
        // TODO: handle PyQuery allocation failure
    }
    return (PyObject*)self;
}

PyObject* PyNodeParentIterator::create(PyFeatures* features, NodeRef node, int startWith)
{
    PyNodeParentIterator* self = (PyNodeParentIterator*)TYPE.tp_alloc(&TYPE, 0);
    if (self)
    {
        self->target = Python::newRef(features);
        // By holding a reference to the PyFeatures, we ensure that the
        // FeatureStore, the matcher and the filter stay alive as long as
        // the PyNodeParentIterator is alive
        self->status = startWith;
        // We either start with RELATIONS (followed by WAYS), 
        // or skip straight to WAYS
        const Filter* secondaryFilter = features->filter;
        if (startWith == IterationStatus::RELATIONS)
        {
            // If we need relations, initialize the iterator
            new(&self->relationIter)ParentRelationIterator(features->store, 
                node.relationTableFast(), features->matcher, secondaryFilter);
        }
        new(&self->featureNodeFilter)FeatureNodeFilter(node, secondaryFilter);
        // We don't change refcount of the secondary filter, since we can
        // be assured that is alive as long as this iterator is alive (because
        // `target` holds a strong reference to Features)
        self->wayQuery = PyQuery::create(features, node.bounds(),
            features->acceptedTypes & 
                (FeatureTypes::WAYS & FeatureTypes::WAYNODE_FLAGGED), 
                features->matcher, &self->featureNodeFilter);
        // We restrain acceptedTypes, because it may include relations
        // (We don't set it to (FeatureTypes::WAYS & FeatureTypes::WAYNODE_FLAGGED),
        // because the acceptedTypes of Features may be stricter, such as
        // "only areas" or "only relation members")
        // TODO: handle PyQuery allocation failure
    }
    return (PyObject*)self;
}



void PyNodeParentIterator::dealloc(PyNodeParentIterator* self)
{
    // Wait for query to complete, otherwise we risk deallocating
    // filters while the query engine threads may still be accessing
    // them
    if (self->status != IterationStatus::DONE)
    {
        while (self->wayQuery->query.next());
        // All possible remainging ways have been returned at 
        // this point and the query has shut down
    }
    Py_DECREF(self->wayQuery);
    Py_DECREF(self->target);
    Py_TYPE(self)->tp_free(self);
}

PyObject* PyNodeParentIterator::next(PyNodeParentIterator* self)
{
    if (self->status == IterationStatus::RELATIONS)
    {
        RelationRef relation = self->relationIter.next();
        if (!relation.isNull())
        {
            return PyFeature::create(self->relationIter.store(), relation, Py_None);
        }
        // No more relations, move on to ways
        self->status = IterationStatus::WAYS;
    }
    // We need to distinguish between "no more features" and 
    // "PyFeature allocation failed", hence we call the underlying
    // query directly
    pointer pFeature = self->wayQuery->query.next();
    if (pFeature)
    {
        return PyFeature::create(self->target->store, pFeature, Py_None);
    }
    // Exhausted all relations and ways, iteration is done
    self->status = IterationStatus::DONE;
    return NULL;
}


PyTypeObject PyNodeParentIterator::TYPE =
{
    .tp_name = "geodesk.NodeParentIterator",
    .tp_basicsize = sizeof(PyNodeParentIterator),
    .tp_dealloc = (destructor)dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT, // | Py_TPFLAGS_DISALLOW_INSTANTIATION,
    .tp_iter = PyObject_SelfIter,
    .tp_iternext = (iternextfunc)next,
};