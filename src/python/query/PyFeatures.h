// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <functional>
#include <Python.h>
#include <geodesk/feature/FeatureNodeIterator.h>
#include <geodesk/feature/WayNodeCursor.h>
#include <geodesk/feature/MemberIterator.h>
#include <geodesk/feature/ParentRelationIterator.h>
#include <geodesk/feature/TypedFeatureId.h>
#include <geodesk/filter/FeatureNodeFilter.h>
#include <geodesk/filter/WayNodeFilter.h>
#include <geodesk/query/ViewBase.h>
#include <geodesk/geom/Box.h>
#include "python/Environment.h"

using namespace geodesk;
namespace geodesk {
class FeatureStore;
class Filter;
class Matcher;
}
class PyAnonymousNode;
class PyFeature;
class PyFeatures;
class PyQuery;

// Must bePyObject to support both PyFeature and PyAnonymousNode
typedef const std::function<void(PyObject* feature)>& FeatureFunction;


// TODO: Unresolved issue: Need to distinguish between way.nodes and way.members:
// The latter uses a NodeIterator (all nodes, including anonymous), the latter 
// FeatureNodeIterator (only feature nodes). way.nodes becomes way.members if 
// a Matcher is applied. Could simply check this corner case direcly, but would be 
// cleaner if we decoupled the logic

// TODO: We could drop selectionType, which brings object size down to 64 bytes
//  (currently, because of allocator padding, size is 80 bytes)

// TODO: We could cache the results of queries for member and nodes
//  (There's room in the union), which allows random access via subscript

class PyFeatures : public PyObject, public ViewBase<PyFeatures>
{
public:
    static PyTypeObject TYPE;
    static PyMappingMethods MAPPING_METHODS;
    static PyNumberMethods NUMBER_METHODS;
    static PySequenceMethods SEQUENCE_METHODS;

    /**
     * The user-callable constructor: Creates a WORLD Selection for the given GOL.
     */
    static PyFeatures* createNew(PyTypeObject* type, PyObject* args, PyObject* kwds);

    static PyFeatures* createEmpty();

    static PyFeatures* getEmpty()
    {
        return Environment::get().getEmptyFeatures();
    }

    static PyFeatures* build(PyObject* args, PyObject* kwds);

    static PyFeatures* create(int kind, int flags, FeatureTypes types,
        const FeatureStore* store, Context context,
        const MatcherHolder* matcher, const Filter* filter)
    {
        auto self = (PyFeatures*)TYPE.tp_alloc(&TYPE, 0);
        if (self) [[likely]]
        {
            ::new (static_cast<Selection*>(self)) Selection(
                kind, flags, types, store, context, matcher, filter);
        }
        else
        {
            matcher->release();
            if (filter) filter->release();
            store->release();
        }
        return self;
    }

    static void dealloc(PyFeatures* self);
    static PyObject* dir(PyFeatures* self, PyObject* args, PyObject* kwargs);
    static PyObject* call(PyFeatures* self, PyObject* args, PyObject* kwargs);
    static PyObject* getattr(PyFeatures* self, PyObject* name);
    static PyObject* iter(PyFeatures* self);
    static PyObject* next(PyFeatures* self);

    // Mapping Methods

    static PyObject* subscript(PyFeatures* self, PyObject* key);

    // Sequence Methods

    static int contains(PyFeatures* self, PyObject* object);

    // Number method

    static PyObject* op_and(PyFeatures* self, PyObject* other);

    // Properties

    static PyObject* area(PyFeatures* self);
    static PyObject* count(PyFeatures* self);
    static PyObject* first(PyFeatures* self);
    static PyObject* guid(PyFeatures* self);
    static PyObject* indexed_keys(PyFeatures* self);
    static PyObject* length(PyFeatures* self);
    static PyObject* list(PyFeatures* self);
    static PyObject* map(PyFeatures* self);
    static PyObject* nodes(PyFeatures* self);
    static PyObject* one(PyFeatures* self);
    static PyObject* properties(PyFeatures* self);
    static PyObject* refcount(PyFeatures* self);
    static PyObject* relations(PyFeatures* self);
    static PyObject* revision(PyFeatures* self);
    static PyObject* shape(PyFeatures* self);
    static PyObject* strings(PyFeatures* self);
    static PyObject* tiles(PyFeatures* self);
    static PyObject* timestamp(PyFeatures* self);
    static PyObject* ways(PyFeatures* self);
    
    // Methods

    static PyObject* auto_load(PyFeatures* self, PyObject* args, PyObject* kwargs);
    static PyObject* explain(PyFeatures* self, PyObject* args, PyObject* kwargs);
    static PyObject* load(PyFeatures* self, PyObject* args, PyObject* kwargs);
    static PyObject* update(PyFeatures* self, PyObject* args, PyObject* kwargs);

    // Lookup by ID

    static PyObject* node(PyFeatures* self, PyObject* args, PyObject* kwargs);
    static PyObject* way(PyFeatures* self, PyObject* args, PyObject* kwargs);
    static PyObject* relation(PyFeatures* self, PyObject* args, PyObject* kwargs);
    PyObject* findById(FeatureType type, PyObject* args, PyObject* kwargs) const;

    int forEach(FeatureFunction func);
    PyObject* getFirst(bool mustHaveOne, bool mayHaveMore) const;
    PyObject* getIter();
    PyObject* getList(Py_ssize_t maxLen);
    static int isTrue(PyFeatures* self);
    bool containsFeature(PyFeature* feature) const;
    bool containsAnonNode(PyAnonymousNode* node) const;
    static PyObject* getTiles(PyFeatures*);

    class Empty;
    class World;
    class WayNodes;
    class Members;
    class Parents;
};


class PyWayNodeIterator : public PyObject
{
public:
    PyObject* target;
    WayNodeCursor nodeCursor;
    FeatureNodeIterator featureIter;
    NodePtr nextNode;
    bool featureNodesOnly;      
        // TODO: We could move this flag into WayCoordinateIterator in
        // order to make the field layout more compact (but it has
        // no impact on the WCI)

    static PyTypeObject TYPE;

    static PyObject* create(PyFeatures* features);
    static PyObject* create(PyFeature* way);
    static void dealloc(PyWayNodeIterator* self);
    static PyObject* next(PyWayNodeIterator* self);
};

class PyMemberIterator : public PyObject
{
public:
    PyObject* target;
    MemberIterator iter;

    static PyTypeObject TYPE;

    static PyObject* create(PyFeatures* features);
    static PyObject* create(PyFeature* rel);
    static void dealloc(PyMemberIterator* self);
    static PyObject* next(PyMemberIterator* self);
};

class PyParentRelationIterator : public PyObject
{
public:
    PyObject* target;
    ParentRelationIterator iter;

    static PyTypeObject TYPE;

    static PyObject* create(PyFeatures* features, clarisma::DataPtr pRelTable);
    static void dealloc(PyParentRelationIterator* self);
    static PyObject* next(PyParentRelationIterator* self);
};


class PyNodeParentIterator : public PyObject
{
public:
    // We always return parent relations first (if node has any),
    // to allow the way query to find parent ways asynchronously
    enum IterationStatus
    {
        RELATIONS = 0,
        WAYS = 1,
        DONE = 2
    };

    PyFeatures* target;
    PyQuery* wayQuery;
    ParentRelationIterator relationIter;
    // TODO: Ensure that ParentRelationIterator does not use a destructor,
    // because it won't be called by dealloc (since we skip relation iteration
    // if the node is not a relation member)
    union
    {
        FeatureNodeFilter featureNodeFilter;
        WayNodeFilter wayNodeFilter;
    };
    int status;
    
    static PyTypeObject TYPE;

    // The lifecycle of Python objects is managed via create/dealloc,
    // a destructor will never be called; TODO: ensure that the objects
    // above are destoryed explicitly, if needed 
    ~PyNodeParentIterator() = delete;

    static PyObject* create(PyFeatures* features, Coordinate wayNodeXY);
    static PyObject* create(PyFeatures* features, NodePtr node, int startWith);
    static void dealloc(PyNodeParentIterator* self);
    static PyObject* next(PyNodeParentIterator* self);
};

