// Copyright (c) 2025 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "python/Environment.h"

class Changeset;
class PyChangedFeature;

class FeatureBuilder
{
public:
    FeatureBuilder(Changeset* changes, GEOSContextHandle_t context) :
        changes_(changes), context_(context) {}

    PyChangedFeature* fromGeometry(const GEOSGeometry* geom);

private:
    PyChangedFeature* fromPoint(const GEOSGeometry* point) const;
    PyChangedFeature* fromLineString(const GEOSGeometry* linestring) const;
    PyChangedFeature* fromPolygon(const GEOSGeometry* polygon) const;
    PyChangedFeature* fromMultiPolygon(const GEOSGeometry* multiPolygon) const;
    PyChangedFeature* fromGeometryCollection(const GEOSGeometry* geom);
    PyObject* nodesFromCoords(const GEOSCoordSequence* coords,
        unsigned int start, unsigned int endExclusive) const;
    bool createLineStringParts(PyObject* list,
        const GEOSGeometry* ring, PyObject* roleString) const;
    bool createPolygonParts(PyObject* list, const GEOSGeometry* polygon) const;

    static constexpr unsigned int MAX_WAY_NODES = 2000;

    Changeset* changes_;
    GEOSContextHandle_t context_;
};
