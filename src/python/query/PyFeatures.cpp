// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyFeatures.h"
#include "feature/GeometryBuilder.h"
#include "filter/ComboFilter.h"
#include "geom/Area.h"
#include "geom/Length.h"
#include "python/Environment.h"
#include "python/feature/PyFeature.h"
#include "python/format/PyFormatter.h"
#include "python/format/PyMap.h"
#include "python/geom/PyBox.h"
#include "python/util/PyFastMethod.h"
#include "PyQuery.h"
#include <common/util/Parser.h>

#include "PyFeatures_attr.cxx"

// steals the refs to matcher and filter
PyFeatures* PyFeatures::createWith(PyFeatures* base, uint32_t flags,
    FeatureTypes acceptedTypes, const Box* bounds,
    const MatcherHolder* matcher, const Filter* filter)
{
    assert(bounds != nullptr);
    PyTypeObject* type = Py_TYPE(base);
    PyFeatures* self = (PyFeatures*)type->tp_alloc(type, 0);
    if (self)
    {
        self->selectionType = base->selectionType;
        FeatureStore* store = base->store;
        store->addref();
        self->store = store;
        self->flags = flags;
        self->acceptedTypes = acceptedTypes;
        self->matcher = matcher;
        self->filter = filter;
        self->bounds = *bounds;      
        // If base does not use bounds, this copies relatedFeature from the union
        // as long as base->bounds is passed as bounds

        // TODO: what happens to relatedFeature??
        // Need to inc refcount!!! no, has no refcount
    }
    return self;
}

PyFeatures* PyFeatures::createEmpty(const MatcherHolder* matcher)
{
    PyFeatures* self = (PyFeatures*)TYPE.tp_alloc(&TYPE, 0);
    if (self)
    {
        self->selectionType = &Empty::SUBTYPE;
        self->store = nullptr;
        self->flags = 0;
        self->acceptedTypes = 0;
        self->matcher = matcher;
        self->filter = nullptr;
        self->relatedFeature = FeatureRef(nullptr);
    }
    return self;
}


PyFeatures* PyFeatures::create(SelectionType* selectionType, FeatureStore* store,
    FeatureRef relatedFeature, FeatureTypes acceptedTypes)
{
    PyFeatures* self = (PyFeatures*)TYPE.tp_alloc(&TYPE, 0);
    if (self)
    {
        self->selectionType = selectionType;
        store->addref();
        self->store = store;
        self->flags = 0;
        self->acceptedTypes = acceptedTypes;
        self->matcher = store->getAllMatcher();
        self->filter = nullptr;
        self->relatedFeature = relatedFeature;
    }
    return self;
}

PyFeatures* PyFeatures::createRelated(PyFeatures* base, SelectionType* selectionType,
    FeatureRef relatedFeature, FeatureTypes acceptedTypes)
{
    acceptedTypes &= base->acceptedTypes;
    if (!acceptedTypes) return getEmpty();
    PyFeatures* self = (PyFeatures*)TYPE.tp_alloc(&TYPE, 0);
    if (self)
    {
        self->selectionType = selectionType;
        self->acceptedTypes = acceptedTypes;
        self->store = base->store;
        self->flags = base->flags &= ~(SelectionFlags::USES_BOUNDS | SelectionFlags::BOUNDS_ACTIVE);
        self->matcher = base->matcher;
        self->filter = base->filter;
        self->relatedFeature = relatedFeature;
        self->store->addref();
        self->matcher->addref();
        if(self->filter) self->filter->addref();

        // TODO: if bbox-constrained, create a bbox filter
    }
    return self;
}



PyFeatures* PyFeatures::createNew(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    const char* fileName;
    if (!PyArg_ParseTuple(args, "s", &fileName))
    {
        return NULL;
    }
    FeatureStore* store;
    try
    {
        store = FeatureStore::openSingle(std::string_view(fileName, strlen(fileName)));
        // TODO: get string_view directly from param
    }
    catch (const FileNotFoundException& ex)
    {
        PyErr_SetString(PyExc_FileNotFoundError, ex.what());
        return NULL;
    }
    catch (const std::bad_alloc&)
    {
        PyErr_NoMemory();
        return NULL;
    }
    catch (const std::exception& ex)
    {
        PyErr_SetString(PyExc_RuntimeError, ex.what());
        return NULL;
    }
    PyFeatures* self = (PyFeatures*)TYPE.tp_alloc(&TYPE, 0);
    if (self)
    {
        self->selectionType = &World::SUBTYPE;
        store->addref();
        self->store = store;
        self->flags = SelectionFlags::USES_BOUNDS;
        self->acceptedTypes = FeatureTypes::ALL;
        self->matcher = store->getAllMatcher();
        self->filter = NULL;
        self->bounds = Box::ofWorld();
    }
    return self;
}


void PyFeatures::dealloc(PyFeatures* self)
{
    self->matcher->release();
    if(self->filter) (self->filter->release());
    if(self->store) self->store->release();
    Py_TYPE(self)->tp_free(self);
}

PyObject* PyFeatures::iter(PyFeatures* self)
{
    return self->selectionType->iter(self);
}

PyObject* PyFeatures::World::iterFeatures(PyFeatures* self)
{
    return PyQuery::create(self);
}


PyObject* PyFeatures::call(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
    Py_ssize_t argCount = PyTuple_Size(args);
    if (argCount == 1)
    {
        // single argument
        PyObject* arg = PyTuple_GetItem(args, 0);
        PyTypeObject* type = Py_TYPE(arg);
        if (type == &PyBox::TYPE)
        {
            Box box = ((PyBox*)arg)->box;
            if (box.isEmpty()) return getEmpty();
            if (self->flags & SelectionFlags::USES_BOUNDS)
            {
                if (self->flags & SelectionFlags::BOUNDS_ACTIVE)
                {
                    box = Box::simpleIntersection(box, self->bounds);
                    if(box.isEmpty()) return getEmpty();
                }
                self->matcher->addref();
                if(self->filter) self->filter->addref();
                return (PyObject*)createWith(self, self->flags | SelectionFlags::BOUNDS_ACTIVE,
                    self->acceptedTypes, &box, self->matcher, self->filter);
            }
            else
            {
                PyErr_SetString(PyExc_NotImplementedError, "Not yet implemented.");
                return NULL;
            }
        }
        if (type == &PyUnicode_Type)
        {
            Py_ssize_t len;
            const char* query = PyUnicode_AsUTF8AndSize(arg, &len);
            if (!query) return NULL;
            return self->withQuery(query);
        }
        if (type == &TYPE)
        {
            return (PyObject*)self->withOther((PyFeatures*)arg);
        }
        if (arg == (PyObject*)Py_None) return getEmpty();

        PyErr_Format(PyExc_TypeError, "%s is not a valid argument", type->tp_name);
    }
    else if (argCount == 0)
    {
        return Python::newRef(self);
    }
    PyErr_SetString(PyExc_TypeError, "Expected query|Box|Features");
    return NULL;
}

// TODO: use negative value to signal error
/*
uint64_t PyFeatures::getCount()
{
    return selectionType->count(this);
}
*/

PyObject* PyFeatures::World::countFeatures(PyFeatures* self) 
{
    int64_t count = 0;
    Query query(self->store, self->bounds, self->acceptedTypes, self->matcher, self->filter);
    while (query.next()) count++;
    // TODO: error handling
    return PyLong_FromLongLong(count);
}

PyObject* PyFeatures::countFeatures(PyFeatures* self)
{
    int64_t count = 0;
    PyObject* iter = self->selectionType->iter(self);
    if (!iter) return 0;
    for (;;)
    {
        PyObject* feature = PyIter_Next(iter);
        if (!feature) break;
        Py_DECREF(feature);
        count++;
    }
    Py_DECREF(iter);
    return PyLong_FromLongLong(count);
}

int PyFeatures::forEach(FeatureFunction func)
{
    PyObject* iter = selectionType->iter(this);
    if (!iter) return -1;
    for (;;)
    {
        PyFeature* feature = (PyFeature*)PyIter_Next(iter);
        if (feature == NULL) break;
        func(feature);
    }
    return PyErr_Occurred() ? -1 : 0;
}


PyObject* PyFeatures::getFirst(bool mustHaveOne, bool mayHaveMore)
{
    PyObject* iter = selectionType->iter(this);
    if (!iter) return NULL;
    PyObject* result = PyIter_Next(iter);
    if (PyErr_Occurred())
    {
        result = NULL;
    }
    else
    {
        if (!result)
        {
            if (mustHaveOne)
            {
                Environment::get().raiseQueryException("No feature found.");
                result = NULL;
            }
            else
            {
                result = Python::newRef(Py_None);
            }
        }
        else
        {
            if (!mayHaveMore)
            {
                PyObject* more = PyIter_Next(iter);
                if (PyErr_Occurred())
                {
                    result = NULL;
                }
                else if(more)
                {
                    Environment::get().raiseQueryException(
                        "Expected only one feature, but found multiple.");
                        // TODO: list them
                    result = NULL;
                }
            }
        }
    }
    Py_DECREF(iter);
    return result;
}

int PyFeatures::isTrue(PyFeatures* self) 
{
    int empty = self->selectionType->isEmpty(self);
    return empty < 0 ? -1 : (empty ? 0 : 1);
}

PyObject* PyFeatures::getattr(PyFeatures* self, PyObject* nameObj)
{
    Py_ssize_t len;
    const char* name = PyUnicode_AsUTF8AndSize(nameObj, &len);
    if (!name) return NULL;

    PyFeaturesAttribute* attr = PyFeatures_AttrHash::lookup(name, len);
    if (attr)
    {
        Python::AttrRef ref = attr->attr;
        if (ref.isMethod())
        {
            return PyFastMethod::create(self, ref.method());
        }
        else
        {
            return ref.getter()(self);
        }
    }
    return PyObject_GenericGetAttr(self, nameObj);
    /*
    PyErr_SetString(PyExc_AttributeError, name);
    return NULL;
    */
}

int PyFeatures::contains(PyFeatures* self, PyObject* other)
{
    // TODO
    return 0;
}

PyObject* PyFeatures::subscript(PyFeatures* self, PyObject* key)
{
    if (PySlice_Check(key))
    {
        Py_ssize_t start, stop, step, len;

        if (PySlice_GetIndicesEx(key, PY_SSIZE_T_MAX, &start, &stop, &step, &len) < 0)
        {
            return NULL;
        }
        if (start != 0)
        {
            PyErr_SetString(PyExc_ValueError, "Slice must start at 0");
            return NULL;
        }
        if (step != 1)
        {
            PyErr_SetString(PyExc_ValueError, "Step size must be 1");
            return NULL;
        }
        return self->getList(len);
    }
    if (PyNumber_Check(key))
    {
        long index = PyLong_AsLong(key);
        if (index == -1 && PyErr_Occurred()) return NULL;
        if (index != 0)
        {
            Environment::get().raiseQueryException("Only [0] is allowed.");
            return NULL;
        }
        return self->getFirst(true /* mustHaveOne */, true /* mayHaveMore */);
    }
    PyErr_SetString(PyExc_TypeError, "Must be [0] or [:max_count]");
    return NULL;
}


PyObject* PyFeatures::getList(Py_ssize_t maxLen)
{
    PyObject* list = PyList_New(maxLen);
    if (!list) return NULL;

    // TODO: use negative value to signal error
    Py_ssize_t count = 0;
    PyObject* iter = selectionType->iter(this);
    if (!iter)
    {
        Py_DECREF(list);
        return NULL;
    }
    while (count < maxLen)
    {
        PyObject* feature = PyIter_Next(iter);
        if (!feature) break;
        PyList_SET_ITEM(list, count++, feature); 
        // Note: PyList_SET_ITEM steals a reference
    }

    Py_DECREF(iter);   // we're done with the query
    if (PyErr_Occurred())
    {
        Py_DECREF(list);
        return NULL;
    }
    
    if (count < maxLen)
    {
        if (PyList_SetSlice(list, count, maxLen, NULL) == -1) 
        {
            Py_DECREF(list);
            return NULL;
        }
    }
    return list;
}


PyObject* PyFeatures::op_and(PyFeatures* self, PyObject* other)
{
    if (Py_TYPE(other) != &TYPE)
    {
        Py_RETURN_NOTIMPLEMENTED;
    }
    return self->withOther((PyFeatures*)other);
}


PyMappingMethods PyFeatures::MAPPING_METHODS =
{
    .mp_subscript = (binaryfunc)PyFeatures::subscript,
};

PyNumberMethods PyFeatures::NUMBER_METHODS =
{
    .nb_bool = (inquiry)isTrue,
    .nb_and = (binaryfunc)op_and,
};

PySequenceMethods PyFeatures::SEQUENCE_METHODS
{
    .sq_contains = (objobjproc)&contains,
};


PyTypeObject PyFeatures::TYPE =
{
    .tp_name = "geodesk.Features",      
    .tp_basicsize = sizeof(PyFeatures),       
    .tp_dealloc = (destructor)PyFeatures::dealloc, 
    .tp_as_number = &NUMBER_METHODS,
    .tp_as_sequence = &SEQUENCE_METHODS,
    .tp_as_mapping = &MAPPING_METHODS,
    .tp_call = (ternaryfunc)PyFeatures::call, 
    .tp_getattro = (getattrofunc)PyFeatures::getattr, 
    .tp_flags = Py_TPFLAGS_DEFAULT,   
    .tp_doc = "Features objects", 
    .tp_iter = (getiterfunc)PyFeatures::iter,  
    .tp_iternext = (iternextfunc)next,
    // .tp_init = (initproc)PyFeatures::init,   
    .tp_new = (newfunc)createNew
};

PyFeatures* PyFeatures::withQuery(const char* query)
{
    try
    {
        const MatcherHolder* newMatcher = store->getMatcher(query);
        uint32_t newTypes = acceptedTypes & newMatcher->acceptedTypes();
        if (newTypes == 0)
        {
            newMatcher->release();
            return getEmpty();
        }

        // TODO: combine newMatcher with any existing matcher
        if(filter) filter->addref();     // need to addref filter because createWith steals the ref
        return createWith(this, flags | USES_MATCHER, newTypes, &bounds, newMatcher, filter);
    }
    catch (const ParseException& ex)
    {
        // Environment::get().raiseQueryException("Bad query: %s", ex.what());
        Environment::get().raiseQueryException(ex.what());
        return NULL;
    }
}


PyFeatures* PyFeatures::withFilter(const Filter* newFilter)
{
    if (filter)
    {
        const ComboFilter* combo = new ComboFilter(filter, newFilter);
        newFilter->release();
            // Need to release because this function is expected to consume the
            // reference (ComboFilter adds its own ref)
        newFilter = combo;
    }
    FeatureTypes newTypes = acceptedTypes & newFilter->acceptedTypes();
    if (newTypes == 0)
    {
        newFilter->release();
        return getEmpty();
    }

    // TODO: apply bounds!!!
    // 
    // TODO: fix
    Box b = newFilter->getBounds();
    // Box b = Box::simpleIntersection(bounds, newFilter->getBounds());

    matcher->addref();      // createWith consumes ref to matcher
    return createWith(this, flags | USES_FILTER, newTypes, 
        (flags & USES_BOUNDS) ? &b : &bounds, 
            matcher, newFilter);
}


PyFeatures* PyFeatures::withOther(PyFeatures* other)
{
    FeatureTypes newTypes = acceptedTypes & other->acceptedTypes;
    if (newTypes == 0) return getEmpty();
    
    // TODO: only allow intersecting World queries for now
    // Others will require special handling, rarely used

    // TODO: merge matcher
    // TODO: merge bounding boxes (need to consider world vs. related query)
    // TODO: enforce same-store rule

    const MatcherHolder* newMatcher;
    if (flags & USES_MATCHER)
    {
        if (other->flags & USES_MATCHER)
        {
            PyErr_SetString(PyExc_NotImplementedError,
                "This type of query will be supported in Version 0.2.0");
            return NULL;
        }
        newMatcher = matcher;
        newMatcher->addref();
    }
    else
    {
        newMatcher = other->matcher;
        newMatcher->addref();
    }

    const Filter* newFilter = other->filter;
    if (newFilter)
    {
        if (filter)
        {
            newFilter = new ComboFilter(filter, newFilter);
        }
        else
        {
            newFilter->addref();
        }
    }
    else if (filter)
    {
        newFilter = filter;
        newFilter->addref();
    }
    else
    {
        newFilter = nullptr;
    }

    return createWith(this, flags | other->flags, newTypes,
        &bounds, newMatcher, newFilter);
}


PyFeatures* PyFeatures::withTypes(FeatureTypes newTypes)
{
    newTypes &= acceptedTypes;
    if (newTypes == 0) return getEmpty();
    matcher->addref();              // matcher can never be null
    if(filter) filter->addref();    // however, filter is allowed to be null
    return createWith(this, flags, newTypes, &bounds, matcher, filter);
}

// PyFeatures itself is an empty iterator, makes it easy for EMPTY Selection
PyObject* PyFeatures::next(PyFeatures* self)
{
    return NULL;
}


/*
PyFeatures* PyFeatures::copy(PyFeatures* base)
{
    PyFeatures* self = (PyFeatures*)TYPE.tp_alloc(&TYPE, 0);
    if (self)
    {
        self->selectionType = base->selectionType;
        FeatureStore* store = base->store;
        store->addref();
        self->store = store;
        self->acceptedTypes = base->acceptedTypes;
        const MatcherHolder* matcher = base->matcher;
        matcher->addref();  
        self->matcher = matcher;
        PyFilter* filter = base->filter;
        Py_XINCREF(filter);
        self->filter = filter;
        self->bounds = base->bounds;  // Copies the entire union
    }
    return self;
}

PyFeatures* PyFeatures::world_newWithBox(PyFeatures* base, const Box& box)
{
    PyFeatures* self = copy(base);
    if (self) self->bounds = box;
    return self;
}
*/

SelectionType PyFeatures::World::SUBTYPE =
{
    iterFeatures,
    countFeatures,
    isEmpty
};


PyObject* PyFeatures::Empty::iterFeatures(PyFeatures* self)
{
    // The PyFeatures itself is an empty iterator
    return Python::newRef(self);
}

PyObject* PyFeatures::Empty::countFeatures(PyFeatures*)
{
    return PyLong_FromLong(0);
}

int PyFeatures::Empty::isEmpty(PyFeatures*)
{
    return 1;
}

// TODO: This is broken !!!!!
int PyFeatures::isEmpty(PyFeatures* self)
{
    PyObject* iter = self->selectionType->iter(self);
    if (iter == NULL) return -1;
    bool isEmpty = PyIter_Next(iter) == NULL;
    if (PyErr_Occurred()) PyErr_Clear();
    Py_DECREF(iter);
    return isEmpty;
}

bool PyFeatures::acceptsAny(FeatureTypes types)
{
    // TODO: can't use this to check if set is unconstrained: needs to test
    // if *all* requested types are accepted (acceptedTypes & types) == types

    return (acceptedTypes & types) != 0 && filter == nullptr &&
        matcher == store->borrowAllMatcher();
    // TODO: all-matcher should really be stored in the Environment
}

SelectionType PyFeatures::Empty::SUBTYPE =
{
    iterFeatures,
    countFeatures,
    isEmpty
};

// === Properties ===

PyObject* PyFeatures::area(PyFeatures* self)
{
    double totalArea = 0;
    int res = self->forEach([&totalArea](PyFeature* feature)
    {
        FeatureRef f = feature->feature;
        if (f.isArea())
        {
            double area;
            if (f.isWay())
            {
                area = Area::ofWay(WayRef(f));
            } 
            else
            {
                assert(f.isRelation());
                area = Area::ofRelation(feature->store, RelationRef(f));
            }
            totalArea += area;
        }
    });
    return res == 0 ? PyFloat_FromDouble(totalArea) : NULL;
}

PyObject* PyFeatures::count(PyFeatures* self)
{
    return self->selectionType->count(self);
}

PyObject* PyFeatures::first(PyFeatures* self)
{
    return self->getFirst(false /* mustHaveOne */, true /* mayHaveMore */);
}

PyObject* PyFeatures::guid(PyFeatures* self)
{
    // TODO
    Py_RETURN_NONE;
}

PyObject* PyFeatures::indexed_keys(PyFeatures* self)
{
    // TODO
    Py_RETURN_NONE;
}

PyObject* PyFeatures::length(PyFeatures* self)
{
    // TODO: Can this be confused with "count"?
    // Make it total_length?
    // Would need to change area to total_area

    double totalLength = 0;
    int res = self->forEach([&totalLength](PyFeature* feature)
    {
        FeatureRef f = feature->feature;
        double length;
        if (f.isWay())
        {
            length = Length::ofWay(WayRef(f));
        }
        else if(f.isRelation())
        {
            length = Length::ofRelation(feature->store, RelationRef(f));
        }
        totalLength += length;
    });
    return res==0 ? PyFloat_FromDouble(totalLength) : NULL;
}

PyObject* PyFeatures::list(PyFeatures* self)
{
    // TODO
    Py_RETURN_NONE;
}

PyObject* PyFeatures::map(PyFeatures* self)
{
    return PyMap::create(self);
}

PyObject* PyFeatures::nodes(PyFeatures* self)
{
    return (PyObject*)self->withTypes(FeatureTypes::NODES);
}

PyObject* PyFeatures::one(PyFeatures* self)
{
    return self->getFirst(true /* mustHaveOne */, false /* mayHaveMore */);
}

PyObject* PyFeatures::properties(PyFeatures* self)
{
    // TODO
    Py_RETURN_NONE;
}

PyObject* PyFeatures::refcount(PyFeatures* self)
{
    // TODO
    Py_RETURN_NONE;
}

PyObject* PyFeatures::relations(PyFeatures* self)
{
    return (PyObject*)self->withTypes(FeatureTypes::RELATIONS);
}

PyObject* PyFeatures::revision(PyFeatures* self)
{
    // TODO
    Py_RETURN_NONE;
}

PyObject* PyFeatures::shape(PyFeatures* self)
{
    Environment& env = Environment::get();
    GEOSContextHandle_t geosContext = env.getGeosContext();
    if (!geosContext) return NULL;

    std::vector< GEOSGeometry*> geoms;
    int res = self->forEach([&geoms, geosContext](PyFeature* feature)
        {
            geoms.push_back(GeometryBuilder::buildFeatureGeometry(
                feature->store, feature->feature, geosContext));
        });
    if (res < 0)
    {
        for (GEOSGeometry* geom : geoms) 
        {
            GEOSGeom_destroy(geom);
        }
        return NULL;
    }
    GEOSGeometry* collection = GEOSGeom_createCollection_r(
        geosContext, GEOS_GEOMETRYCOLLECTION, geoms.data(), 
        static_cast<unsigned int>(geoms.size()));
    return env.buildShapelyGeometry(collection);
}

PyObject* PyFeatures::strings(PyFeatures* self)
{
    // TODO
    Py_RETURN_NONE;
}

PyObject* PyFeatures::tiles(PyFeatures* self)
{
    // TODO
    Py_RETURN_NONE;
}

PyObject* PyFeatures::timestamp(PyFeatures* self)
{
    // TODO
    Py_RETURN_NONE;
}

PyObject* PyFeatures::ways(PyFeatures* self)
{
    return (PyObject*)self->withTypes(FeatureTypes::WAYS);
}


PyObject* PyFeatures::auto_load(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
    PyErr_SetString(PyExc_NotImplementedError,
        "auto_load will be available in Version 0.2.0");
    return NULL;
}

PyObject* PyFeatures::load(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
    PyErr_SetString(PyExc_NotImplementedError,
        "load will be available in Version 0.2.0");
    return NULL;
}

PyObject* PyFeatures::update(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
    PyErr_SetString(PyExc_NotImplementedError,
        "update will be available in Version 0.2.0");
    return NULL;
}
