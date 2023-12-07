// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyFeatures.h"
#include <unordered_set>
#include "python/feature/PyFeature.h"
#include "query/TileIndexWalker.h"
#include "PyTile.h"


PyObject* PyFeatures::Members::iterFeatures(PyFeatures* features)
{
    return PyMemberIterator::create(features);
}

int PyFeatures::Members::isEmpty(PyFeatures* features)
{
    // TODO: can shortcut if non-filtered
    return PyFeatures::isEmpty(features);
}

PyObject* PyFeatures::Members::getTiles(PyFeatures* self)
{
    std::unordered_set<uint32_t> tips;
    FeatureStore* store = self->store;
    RelationRef relation(self->relatedFeature);
    MemberIterator iter(store, relation.bodyptr(),
        self->acceptedTypes, self->matcher, self->filter);
    for(;;)
    {
        FeatureRef member = iter.next();
        if (member.isNull()) break;
        if (iter.isCurrentForeign()) tips.insert(iter.currentTip());
    }

    PyObject* list = PyList_New(0);
    if (list)
    {
        FeatureStore* store = self->store;

        // We have to use a TIW in order to obtain tile numbers from the set
        // of tips; we constrain the walker to the bbox of the relation, since
        // by definition member tiles cannot lie outside of the relation's
        // bounds (TODO: non-spatial members?)

        TileIndexWalker tiw(store->tileIndex(), store->zoomLevels(), relation.bounds());
            // TODO: could calculate tighter bounds based on accepted members
        while (tiw.next())
        {
            Tip tip = tiw.currentTip();
            if (tips.find(tip) == tips.end()) continue;
            PyTile* tile = PyTile::create(store, tiw.currentTile(), tip);
            if (tile)
            {
                if (PyList_Append(list, tile) == 0)
                {
                    Py_DECREF(tile);
                    continue;
                }
                Py_DECREF(tile);
            }
            Py_DECREF(list);
            return NULL;
        }
    }
    return list;
}


SelectionType PyFeatures::Members::SUBTYPE =
{
    iterFeatures,
    countFeatures,
    isEmpty,
    containsFeature,
    getTiles
};


// TODO: containsFeature should check for member flag to rule out given feature

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


