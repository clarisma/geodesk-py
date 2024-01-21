// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyQuery.h"
#include "python/Environment.h"
#include "python/feature/PyFeature.h"
#include "PyFeatures.h"
#include "PyQueryFinalizer.h"

PyQuery* PyQuery::create(PyFeatures* features)
{
    LOG("PyQuery::create(PyFeatures*)");
    PyQuery* self = (PyQuery*)TYPE.tp_alloc(&TYPE, 0);
    if (self != nullptr)
    {
        Py_INCREF(features);
        self->target = features;
        // initialize Query in-place
        new(&self->query)Query(
            features->store,
            features->bounds,
            features->acceptedTypes,
            features->matcher,
            features->filter);
    }
    return self;
}


PyQuery* PyQuery::create(PyFeatures* features,
    const Box& box, FeatureTypes types,
    const MatcherHolder* matcher, const Filter* filter)
{
    PyQuery* self = (PyQuery*)TYPE.tp_alloc(&TYPE, 0);
    if (self != nullptr)
    {
        Py_INCREF(features);
        self->target = features;
        // initialize Query in-place
        new(&self->query)Query(features->store, box, types, matcher, filter);
    }
    return self;
}



void PyQuery::dealloc(PyQuery* self)
{
    LOG("Deallocating PyQuery...");

    // Check if query is still running, and if so, attempt to cancel it
    // (but don't await its completion)
    // If the query is still running, we enqueue it in the QueryFinalizer
    // The QueryFinalizer runs whenever garbage collection occurs, or when
    // a FeatureStore is about to be deallocated
    // When we enqueue a PyQuery for finalization, we drop its reference
    // to PyFeatures (so it no longer indirectly references the FeatureStore),
    // but we add references to its Matcher and Filter (to ensure they are 
    // still available to the running query). If the store's refcount drops
    // to zero, it will ask the QueryFinalizer to await completion of all
    // queries on this store before closing the store file and deallocating itself

    // TODO: temporaily disabled deallocation until we implement the above

    assert(Py_REFCNT(self) == 0);

    if (false /* TODO: tryCancel() */)
    {
        Py_XDECREF(self->target);       // target may have been set to null
                                        // during finalization
        self->query.~Query();           // call destructor explicitly
        Py_TYPE(self)->tp_free(self);
    }
    else
    {
        // We need to save any currently pending exception
        // Creating the finalizer (or even just enqueing the query) may cause 
        // other exceptions to be raised

        PyObject* errorType, * errorValue, * traceback;
        PyErr_Fetch(&errorType, &errorValue, &traceback);
        PyErr_Clear();

        PyQueryFinalizer* finalizer = Environment::get().getQueryFinalizer();
        if (!finalizer)
        {
            Environment::clearAndLogException();
            // Something went seriously wrong and we can't do anything,
            // so we'll just let the PyQuery leak
        }
        else
        {
            Py_INCREF(self); // resurrect this object
            assert(Py_REFCNT(self) == 1);
            finalizer->track(self);
        }

        // Restore the previous exception (if any)
        PyErr_Restore(errorType, errorValue, traceback);
    }
}


PyObject* PyQuery::iter(PyQuery* self)
{
    Py_INCREF(self);
    return reinterpret_cast<PyObject*>(self);
}

PyObject* PyQuery::next(PyQuery* self)
{
    // LOG("PyQuery::next()");
    pointer pFeature = self->query.next();
    if (pFeature)
    {
        return PyFeature::create(self->query.store(), pFeature, Py_None);
    }
    return nullptr;     
}


PyTypeObject PyQuery::TYPE =
{
    PyVarObject_HEAD_INIT(nullptr, 0)
    "geodesk.Query",      // tp_name 
    sizeof(PyQuery),      // tp_basicsize 
    0, // tp_itemsize 
    reinterpret_cast<destructor>(PyQuery::dealloc), // tp_dealloc 
    0, // tp_print 
    0, // tp_getattr 
    0, // tp_setattr 
    0, // tp_reserved 
    0, // tp_repr 
    0, // tp_as_number 
    0, // tp_as_sequence 
    0, // tp_as_mapping 
    0, // tp_hash 
    0, // tp_call 
    0, // tp_str 
    0, // tp_getattro 
    0, // tp_setattro 
    0, // tp_as_buffer 
    Py_TPFLAGS_DEFAULT, // | Py_TPFLAGS_DISALLOW_INSTANTIATION, // tp_flags 
    "Query objects", // tp_doc 
    0, // tp_traverse 
    0, // tp_clear 
    0, // tp_richcompare 
    0, // tp_weaklistoffset 
    reinterpret_cast<getiterfunc>(PyQuery::iter), // tp_iter 
    reinterpret_cast<iternextfunc>(PyQuery::next), // tp_iternext 
    0, // tp_methods 
    0, // tp_members 
    0, // tp_getset 
    0, // tp_base 
    0, // tp_dict 
    0, // tp_descr_get 
    0, // tp_descr_set 
    0, // tp_dictoffset 
    0, // tp_init 
    0, // tp_alloc 
    0, // tp_new 
};

