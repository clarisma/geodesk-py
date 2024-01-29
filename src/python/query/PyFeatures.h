// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <functional>
#include <Python.h>
#include "python/Environment.h"
#include "feature/Way.h"
#include "feature/MemberIterator.h"
#include "feature/ParentRelationIterator.h"
#include "geom/Box.h"

class FeatureStore;
class Filter;
class Matcher;
class PyAnonymousNode;
class PyFeature;
class PyFeatures;
class PyQuery;

// Must bePyObject to support both PyFeature and PyAnonymousNode
typedef const std::function<void(PyObject* feature)>& FeatureFunction;

struct SelectionType
{
    PyObject* (*iter)(PyFeatures*);
    PyObject* (*count)(PyFeatures*);      
    int (*isEmpty)(PyFeatures*);    
    int (*containsFeature)(PyFeatures*, PyObject*);
    PyObject* (*getTiles)(PyFeatures*);
};

enum SelectionFlags
{
    /**
     * The PyFeatures instance uses `bounds` (instead of `relatedFeature` in
     * the same union). Only used by WORLD selection.
     */
    USES_BOUNDS = 1,
    /**
     * `bounds` has bounds tighter than `Box::ofWorld`. `USES_BOUNDS` must be set.
     */
    BOUNDS_ACTIVE = 2,
    USES_MATCHER = 4,
    USES_FILTER = 8,

    // TODO: need flag to indicate if relatedFeature is in use
    // or does NOT USES_BOUNDS imply use of relatedFeature?
};

// TODO: Unresolved issue: Need to distinguish between way.nodes and way.members:
// The latter uses a NodeIterator (all nodes, including anonymous), the latter 
// FeatureNodeIterator (only feature nodes). way.nodes becomes way.members if 
// a Matcher is applied. Could simply check this corner case direcly, but would be 
// cleaner if we decoupled the logic

class PyFeatures : public PyObject
{
public:
    // PyObject_HEAD
    SelectionType* selectionType;
    FeatureStore* store;
    FeatureTypes acceptedTypes;
    uint32_t flags;
    const MatcherHolder* matcher;
    const Filter* filter;
    union
    {
        Box bounds;                 // If used, USES_BOUNDS flag must be set 
                                    // If it contains a value other than Box::ofWorld(),
                                    // ACTIVE_BOUNDS must be set
        FeatureRef relatedFeature;  // If used, USES_BOUNDS flag must be clear 
    };

    static PyTypeObject TYPE;
    static PyMappingMethods MAPPING_METHODS;
    static PyNumberMethods NUMBER_METHODS;
    static PySequenceMethods SEQUENCE_METHODS;

    /**
     * The user-callable constructor: Creates a WORLD Selection for the given GOL.
     */
    static PyFeatures* createNew(PyTypeObject* type, PyObject* args, PyObject* kwds);
    /**
     * Creates an unconstrained selection for Features related to the given Feature:
     * MEMBERS, NODES, FEATURE_NODES, PARENTS, PARENT_WAYS, PARENT_RELATIONS
     */
    static PyFeatures* create(SelectionType* selectionType, FeatureStore* store,
        FeatureRef relatedFeature, FeatureTypes acceptedTypes);
    static PyFeatures* createRelated(PyFeatures* base, SelectionType* selectionType, 
        FeatureRef relatedFeature, FeatureTypes acceptedTypes);
    static PyFeatures* createEmpty(const MatcherHolder* matcher);

    /**
     * Creates a new PyFeatures instance based on an existing one.
     * 
     * @param base          the PyFeatures to use as a basis
     * @param flags         the new flags
     * @param acceptedTypes the new types
     * @param bounds        pointer to new bounds, or existing (cannot be NULL,
     *                      even if USES_BOUNDS is not set)
     * @param matcher       the MatcherHolder (cannot be NULL)
     * @param filter        the Filter (may be NULL)
     */
    static PyFeatures* createWith(PyFeatures* base, uint32_t flags,
        FeatureTypes acceptedTypes,  const Box* bounds, 
        const MatcherHolder* matcher, const Filter* filter);

    static PyFeatures* getEmpty()
    {
        return Environment::get().getEmptyFeatures();
    }

    /*
    bool hasMatcher() const
    {
        return matcher != store->borrowAllMatcher();
    }
    */

    static void dealloc(PyFeatures* self);
    static PyObject* dir(PyFeatures* self, PyObject* args, PyObject* kwargs);
    static PyObject* call(PyFeatures* self, PyObject* args, PyObject* kwargs);
    static PyObject* getattr(PyFeatures* self, PyObject* name);
    static PyObject* iter(PyFeatures* self);
    static PyObject* next(PyFeatures* self);

    PyFeatures* withQuery(const char* query);
    PyFeatures* withFilter(const Filter* filter);
    PyFeatures* withTypes(FeatureTypes newTypes);
    PyFeatures* withOther(PyFeatures* other);

    // Selection Methods

    static PyObject* countFeatures(PyFeatures*);
    static int       isEmpty(PyFeatures*);

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
    static PyObject* load(PyFeatures* self, PyObject* args, PyObject* kwargs);
    static PyObject* update(PyFeatures* self, PyObject* args, PyObject* kwargs);

    int forEach(FeatureFunction func);
    PyObject* getFirst(bool mustHaveOne, bool mayHaveMore);
    PyObject* getList(Py_ssize_t maxLen);
    static int isTrue(PyFeatures* self);
    static int containsFeature(PyFeatures* self, PyObject* object);
    static PyObject* getTiles(PyFeatures*);

    /**
     * Checks whether this feature set is only constrained by type (i.e. it
     * does not select based on matcher and/or filter).
     * For Nodes/Members and Parents, it is possible to shortcut countFeatures()
     * and isEmpty() in that case.
     */
    bool acceptsAny(FeatureTypes types);
    bool acceptsAny() { return acceptsAny(FeatureTypes::ALL); }

    class Empty;
    class World;
    class WayNodes;
    class Members;
    class Parents;
};

class PyFeatures::Empty : public PyFeatures
{
public:
    static SelectionType SUBTYPE;
    static PyObject* iterFeatures(PyFeatures*);
    static PyObject* countFeatures(PyFeatures*);
    static int       isEmpty(PyFeatures*);
};

class PyFeatures::World : public PyFeatures
{
public:
    static SelectionType SUBTYPE;
    static PyObject* iterFeatures(PyFeatures*);
    static PyObject* countFeatures(PyFeatures*);
    static int containsFeature(PyFeatures* self, PyObject* feature);
    static PyObject* getTiles(PyFeatures*);
};

class PyFeatures::WayNodes : public PyFeatures
{
public:
    static SelectionType SUBTYPE;
    PyFeatures* createRelated(PyFeatures* base, WayRef way);
    static PyObject* iterFeatures(PyFeatures*);
    static PyObject* countFeatures(PyFeatures*);
    static int       isEmpty(PyFeatures*);
};

class PyFeatures::Members : public PyFeatures
{
public:
    static SelectionType SUBTYPE;
    static PyObject* iterFeatures(PyFeatures*);
    static int       isEmpty(PyFeatures*);
    static PyObject* getTiles(PyFeatures*);
};

class PyFeatures::Parents : public PyFeatures
{
public:
    static SelectionType SUBTYPE;
    static PyFeatures* create(PyAnonymousNode* relatedNode);
    static PyFeatures* create(PyFeatures* base, PyAnonymousNode* relatedNode);
    static PyObject* iterFeatures(PyFeatures*);
    static int       isEmpty(PyFeatures*);
};

class PyWayNodeIterator : public PyObject
{
public:
    PyObject* target;
    WayCoordinateIterator coordsIter;
    FeatureNodeIterator featureIter;
    NodeRef nextNode;
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

    static PyObject* create(PyFeatures* features, pointer pRelTable);
    static void dealloc(PyParentRelationIterator* self);
    static PyObject* next(PyParentRelationIterator* self);
};


class FeatureNodeFilter : public Filter
{
public:
    // We don't need to set acceptedTypes because this Filter
    // will never be combined with others; it is only used for
    // finding ways that contain a specific node
    FeatureNodeFilter(NodeRef node, const Filter* filter) :
        node_(node),
        secondaryFilter_(filter)
    {
    }

    bool accept(FeatureStore* store, FeatureRef feature, FastFilterHint fast) const override;

private:
    NodeRef node_;
    const Filter* secondaryFilter_;
};


class WayNodeFilter : public Filter
{
public:
    // We don't need to set acceptedTypes because this Filter
    // will never be combined with others; it is only used for
    // finding ways that contain a specific node
    WayNodeFilter(Coordinate coord, const Filter* filter) :
        coord_(coord),
        secondaryFilter_(filter)
    {
    }

    bool accept(FeatureStore* store, FeatureRef feature, FastFilterHint fast) const override;

private:
    Coordinate coord_;
    const Filter* secondaryFilter_;
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
    static PyObject* create(PyFeatures* features, NodeRef node, int startWith);
    static void dealloc(PyNodeParentIterator* self);
    static PyObject* next(PyNodeParentIterator* self);
};

