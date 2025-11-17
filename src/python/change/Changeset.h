// Copyright (c) 2025 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include "PyChangedFeature.h"
#include <clarisma/data/HashMap.h>
#include <geodesk/geom/Coordinate.h>
#include <geodesk/geom/FixedLonLat.h>
#include "python/util/PythonRef.h"

using PyFeatureRef = PythonRef<PyChangedFeature>;

class Changeset
{
public:
    explicit Changeset(PyObject* tags);

    // TODO: Make threadsafe for multithreaded Python
    void addref() const { ++refcount_; }
    void release() const
    {
        if (--refcount_ == 0)
        {
            delete this;
        }
    }

    void clear();

    PyChangedFeature* createNode(FixedLonLat lonLat);
    PyChangedFeature* createNode(PyObject* first, PyObject* second);
    PyChangedFeature* createWay(PyObject* nodeList);
    PyChangedFeature* createRelation(PyObject* memberList);
    PyChangedFeature* modify(FeatureStore* store, uint64_t id, Coordinate xy);
    PyChangedFeature* modify(PyFeature* feature);
    PyChangedFeature* modify(PyAnonymousNode* feature);

    PyObject* borrowOuter() const { return outerString_.get(); }
    PyObject* borrowInner() const { return innerString_.get(); }

private:
    void addCreated(PyChangedFeature* f);

    mutable size_t refcount_;
    bool live_ = true;
    clarisma::HashMap<FixedLonLat, PyFeatureRef> createdAnonNodes_;
    clarisma::HashMap<Coordinate, PyFeatureRef> existingAnonNodes_;
    std::vector<PyFeatureRef> created_[3];
    clarisma::HashMap<uint64_t, PyFeatureRef> existing_[3];
    PyObjectRef tags_;              // dictionary of changeset tags
    PyObjectRef outerString_;
    PyObjectRef innerString_;

    friend class ChangeWriter;
};
