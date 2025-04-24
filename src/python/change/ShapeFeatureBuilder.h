// Copyright (c) 2025 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "python/Environment.h"

class PyChanges;
class PyChangedFeature;

class ShapeFeatureBuilder
{
public:
    ShapeFeatureBuilder(PyChangedFeature* feature);
    bool fromGeometry(const GEOSGeometry* geom);

private:
    bool fromPoint(const GEOSGeometry* point);
    bool fromLineString(const GEOSGeometry* linestring);
    bool fromPolygon(const GEOSGeometry* polygon);
    bool fromMultiPolygon(const GEOSGeometry* multiPolygon);
    bool fromGeometryCollection(const GEOSGeometry* geom);
    PyObject* nodesFromCoords(const GEOSCoordSequence* coords,
        unsigned int start, unsigned int endExclusive);
    bool createLineStringParts(PyObject* list,
        const GEOSGeometry* ring, PyObject* roleString);
    bool createPolygonParts(PyObject* list, const GEOSGeometry* polygon);

    static constexpr unsigned int MAX_WAY_NODES = 2000;

    PyChanges* changes_;
    PyChangedFeature* feature_;
    GEOSContextHandle_t context_;
};
