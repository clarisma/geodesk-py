// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <Python.h>
#include <geodesk/feature/FeatureStore.h>
#include <geodesk/feature/MemberIterator.h>

using namespace geodesk;
class PyFeature;
class PyFeatures;

typedef PyObject* (*AttrFunctionPtr)(PyFeature* self);

class PyFeature : public PyObject
{
public:
    FeatureStore* store;
    FeaturePtr feature;
    PyObject* roleString;

    static PyTypeObject TYPE;
    static PyTypeObject* SUBTYPES[];
    static PyMappingMethods MAPPING_METHODS;
    static const AttrFunctionPtr* SUBTYPE_FEATURE_METHODS[];

    static PyFeature* create(FeatureStore* store, FeaturePtr feature, PyObject* role);
    static void dealloc(PyFeature* self);
    static PyObject* getattr(PyFeature* self, PyObject* name);
    static PyObject* getattr0(PyFeature* self, PyObject* nameObj, const AttrFunctionPtr* const table);
    static PyObject* getBuiltinAttr(PyFeature* self, PyObject* nameObj, const AttrFunctionPtr* const table);
    static Py_hash_t hash(PyFeature* self);
    static PyObject* iter(PyFeature* self);
    static PyObject* richcompare(PyFeature* self, PyObject* other, int op);
    static int setattr(PyFeature* self, PyObject* name, PyObject* value);
    static PyObject* str(PyFeature* self);
    static PyObject* subscript(PyFeature* self, PyObject* key);

    // Default-Value Getters

    static PyObject* return_true(PyFeature* self);
    static PyObject* return_false(PyFeature* self);
    static PyObject* return_zero(PyFeature* self);
    static PyObject* return_none(PyFeature* self);
    static PyObject* return_blank(PyFeature* self);
    static PyObject* return_empty(PyFeature* self);

    // Methods

    static PyObject* numTagValue(PyFeature* self, PyObject* args, PyObject* kwargs);
    static PyObject* strTagValue(PyFeature* self, PyObject* args, PyObject* kwargs);

    // Properties

    static PyObject* bounds(PyFeature* self);
    static PyObject* id(PyFeature* self);
    static PyObject* is_area(PyFeature* self);
    static PyObject* lat(PyFeature* self);
    static PyObject* lon(PyFeature* self);
    static PyObject* map(PyFeature* self);
    static PyObject* num_method(PyFeature* self);
    static PyObject* osm_type(PyFeature* self);
    static PyObject* parents(PyFeature* self);
    static PyObject* role(PyFeature* self);
    static PyObject* str_method(PyFeature* self);
    static PyObject* tags(PyFeature* self);
    static PyObject* x(PyFeature* self);
    static PyObject* y(PyFeature* self);

    class Node;
    class Way;
    class Relation;
};

class PyFeature::Node : public PyFeature
{
public:
    static PyObject* bounds(PyFeature* self);
    static PyObject* centroid(PyFeature* self);
    static PyObject* is_placeholder(PyFeature* self);
    static PyObject* lat(PyFeature* self);
    static PyObject* lon(PyFeature* self);
    static PyObject* parents(PyFeature* self);
    static PyObject* shape(PyFeature* self);
    static PyObject* type(PyFeature* self);
    static PyObject* x(PyFeature* self);
    static PyObject* y(PyFeature* self);

    static const AttrFunctionPtr FEATURE_METHODS[];
};

class PyFeature::Way : public PyFeature
{
public:
    static PyObject* area(PyFeature* self);
    static PyObject* centroid(PyFeature* self);
    static PyObject* is_placeholder(PyFeature* self);
    static PyObject* length(PyFeature* self);
    static PyObject* nodes(PyFeature* self);
    static PyObject* shape(PyFeature* self);

    static const AttrFunctionPtr FEATURE_METHODS[];
};

class PyFeature::Relation : public PyFeature
{
public:
    static PyObject* area(PyFeature* self);
    static PyObject* centroid(PyFeature* self);
    static PyObject* is_placeholder(PyFeature* self);
    static PyObject* length(PyFeature* self);
    static PyObject* members(PyFeature* self);
    static PyObject* shape(PyFeature* self);

    static const AttrFunctionPtr FEATURE_METHODS[];
};

// Even though PyAnonymousNode inherits from PyFeature,
// the strucutre is different (TODO: check if this creates problems)
class PyAnonymousNode : public PyObject
{
public:
    FeatureStore* store;
    int32_t x_;             // TODO: change to x/y (no underscore)
    int32_t y_;

    static PyTypeObject TYPE;
    static PyMappingMethods MAPPING_METHODS;
    static const AttrFunctionPtr FEATURE_METHODS[];

    static PyObject* create(FeatureStore* store, int32_t x, int32_t y);
    static void dealloc(PyAnonymousNode* self);
    static PyObject* getattr(PyAnonymousNode* self, PyObject* name);
    static Py_hash_t hash(PyAnonymousNode* self);
    static PyObject* richcompare(PyAnonymousNode* self, PyObject* other, int op);
    static PyObject* str(PyAnonymousNode* self);
    static PyObject* subscript(PyAnonymousNode* self, PyObject* key);

    // These are different from return_zero and return_blank, since they
    // are callable with args and kwards and hence can be used as a fucntion
    // for PyFastMethod
    static PyObject* zeroValue(PyFeature* self, PyObject* args, PyObject* kwargs);
    static PyObject* blankValue(PyFeature* self, PyObject* args, PyObject* kwargs);


    static PyObject* bounds(PyAnonymousNode* self);
    static PyObject* centroid(PyAnonymousNode* self);
    static PyObject* lat(PyAnonymousNode* self);
    static PyObject* lon(PyAnonymousNode* self);
    static PyObject* num_method(PyFeature* self);
    static PyObject* osm_type(PyFeature* self);
    static PyObject* parents(PyAnonymousNode* self);
    static PyObject* shape(PyAnonymousNode* self);
    static PyObject* str_method(PyFeature* self);
    static PyObject* tags(PyAnonymousNode* self);
    static PyObject* x(PyAnonymousNode* self);
    static PyObject* y(PyAnonymousNode* self);
};


