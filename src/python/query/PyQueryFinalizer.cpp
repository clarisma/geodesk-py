// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyQueryFinalizer.h"
#include "PyQuery.h"
#include "feature/FeatureStore.h"

#include <common/util/log.h>

PyQueryFinalizer* PyQueryFinalizer::create()
{
    PyQueryFinalizer* self = (PyQueryFinalizer*)(TYPE.tp_alloc(&TYPE, 0));
    if (self)
    {
        new (&self->queries) std::vector<PyQuery*>();
        PyObject* gc_callbacks = getGarbageCollectorCallbacks();
        if (!gc_callbacks)
        {
            Py_DECREF(self);
            return NULL;
        }
        if (!PyList_Check(gc_callbacks))
        {
            PyErr_SetString(PyExc_TypeError, "Expected gc.callbacks to be a list");
            Py_DECREF(self);
            return NULL;
        }
        if(PyList_Append(gc_callbacks, (PyObject*)self) < 0)
        {
            Py_DECREF(self);
            return NULL;
        }
    }
    return self;
}

PyObject* PyQueryFinalizer::call(PyQueryFinalizer* self, PyObject* args, PyObject* kwargs)
{
    // TODO: Check if any tracked queries have completed (but don't wait for them)
    // Queries that have completed are disposed (since we bumped the refcount
    // of their Matcher and Filter before enqueuing them, we also drop refs to those)

    LOG("Attempting to finalize %llu queries...", self->queries.size());
    self->attemptCompletion();
    Py_RETURN_NONE;
}

void PyQueryFinalizer::attemptCompletion()
{
    for (auto it = queries.begin(); it != queries.end(); /* no increment here */)
    {
        PyQuery* query = *it;
        if (false /* TODO: check if query completed */)
        {
            std::iter_swap(it, queries.end() - 1); // swap the current item with the last one
            queries.pop_back(); // remove the last item
            dispose(query);
            // do not increment the iterator here, as we want to process the swapped element on the next loop iteration
        }
        else 
        {
            ++it; // if no removal, simply move to the next item
        }
    }
}

void PyQueryFinalizer::dealloc(PyQueryFinalizer* self)
{
    self->awaitCompletion(nullptr);
    self->queries.~vector();
    Py_TYPE(self)->tp_free((PyObject*)self);
}

PyObject* PyQueryFinalizer::str(PyQueryFinalizer* self)
{
    return PyUnicode_FromFormat("QueryFinalizer (queries=%d)", self->queries.size());
}


void PyQueryFinalizer::track(PyQuery* query)
{
    // A PyQuery that becomes a "zombie" (no longer referenced, but still running)
    // loses its reference to PyFeatures. However, we need to ensure that
    // its Matcher and any Filter are being kept alive, so before we decref and null
    // the PyFeatures reference, we add references to Matcher and Filter (if non-null)
    // When the PyQuery finally dies, we decref thoese resources.
    // Remember, we don't addref the FeatureStore; a FeatureStore with zombie queries
    // will have a PyQueryFinalizer. Once a store's refcount drops to zero, it checks
    // if it has a PyQueryFinalizer, and waits for all of its queries to terminate.
    // Only at that point, FeatureStore deallocates (and removes the PyQueryFinalizer
    // from gc.callbacks)

    // TODO

    assert(Py_REFCNT(query) == 1);

    queries.push_back(query);
    // TODO: This can throw because of out of memory!
}


void PyQueryFinalizer::dispose(PyQuery* query)
{
    // release the query's MatcherHolder
    // release the query's PyFilter (if any)

    assert(Py_REFCNT(query) == 1);
    Py_DECREF(query);
}

/**
 * Awaits completetion of enqueued queries.
 * 
 * @param store (if non-null, only awaits query completion for the specific store)
 */
void PyQueryFinalizer::awaitCompletion(FeatureStore* store)
{
    // Queries that have completed are disposed (since we bumped the refcount
    // of their Matcher and Filter before enqueuing them, we also drop refs to those)

    LOG("Awaiting completion of %ld queries...", queries.size());

    for (auto it = queries.begin(); it != queries.end(); /* no increment here */)
    {
        PyQuery* query = *it;
        if (store == nullptr || store == query->query.store())
        {
            // TODO: await completetion
            std::iter_swap(it, queries.end() - 1); // swap the current item with the last one
            queries.pop_back(); // remove the last item
            dispose(query);
            // do not increment the iterator here, as we want to process the swapped element on the next loop iteration
        }
        else
        {
            ++it; // if no removal, simply move to the next item
        }
    }
}

PyObject* PyQueryFinalizer::getGarbageCollectorCallbacks()
{
    PyObject* gc_module = PyImport_ImportModule("gc");
    if (!gc_module) return NULL;
    PyObject* callbacks_list = PyObject_GetAttrString(gc_module, "callbacks");
    Py_DECREF(gc_module);
    return callbacks_list;
}

PyTypeObject PyQueryFinalizer::TYPE =
{
    .tp_name = "geodesk.QueryFinalizer",
    .tp_basicsize = sizeof(PyQueryFinalizer),
    .tp_dealloc = (destructor)dealloc,
    .tp_repr = (reprfunc)str,
    .tp_call = (ternaryfunc)call,
    .tp_str = (reprfunc)str,
    .tp_flags = Py_TPFLAGS_DEFAULT, // | Py_TPFLAGS_DISALLOW_INSTANTIATION,
    .tp_doc = "QueryFinalizer objects",
};
