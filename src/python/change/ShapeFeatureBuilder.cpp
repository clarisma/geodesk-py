// Copyright (c) 2025 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "ShapeFeatureBuilder.h"
#include "PyChangedFeature.h"
#include "PyChanges.h"
#include "python/geom/PyMercator.h"

bool ShapeFeatureBuilder::fromGeometry(const GEOSGeometry* geom)
{
    switch (GEOSGeomTypeId_r(context_, geom))
    {
    case GEOS_POINT:
        return fromPoint(geom);
    case GEOS_LINESTRING:
    case GEOS_LINEARRING:
        return fromLineString(geom);
    case GEOS_POLYGON:
        return fromPolygon(geom);
    case GEOS_MULTIPOLYGON:
        return fromMultiPolygon(geom);
    case GEOS_GEOMETRYCOLLECTION:
        return fromGeometryCollection(geom);
    default:
        PyErr_SetString(PyExc_ValueError, "Unsupported geometry type");
        return false;
    }
}


bool ShapeFeatureBuilder::fromPoint(const GEOSGeometry* point)
{
    if (feature_->type != PyChangedFeature::NODE)
    {
        if (feature_->type != PyChangedFeature::UNASSIGNED)
        {
            PyErr_SetString(PyExc_ValueError, "Point can only be set for node");
            return false;
        }
        feature_->type = PyChangedFeature::NODE;
    }
    const GEOSCoordSequence* coords = GEOSGeom_getCoordSeq_r(context_, point);
    double xOrLon = 0;
    double yOrLat = 0;
    GEOSCoordSeq_getXY_r(context_, coords, 0, &xOrLon, &yOrLat);
    FixedLonLat lonLat = PyMercator::getAgnosticLonLat(xOrLon,yOrLat);
    feature_->lon = lonLat.lon100nd();
    feature_->lat = lonLat.lat100nd();
    return true;
}

PyObject* ShapeFeatureBuilder::nodesFromCoords(const GEOSCoordSequence* coords,
    unsigned int start, unsigned int endExclusive)
{
    assert(coords);
    assert (start >= 0 && endExclusive > start);
    Py_ssize_t count = endExclusive - start;
    PyObject* list = PyList_New(count);
    if (!list) return nullptr;

    for (unsigned int i = start, j = 0; i < endExclusive; ++i, ++j)
    {
        double x = 0;
        double y = 0;
        GEOSCoordSeq_getXY_r(context_, coords, i, &x, &y);
        FixedLonLat lonLat = PyMercator::getAgnosticLonLat(x, y);
        PyChangedFeature* node = changes_->createNode(lonLat);
        if (!node)
        {
            Py_DECREF(list);
            return nullptr;
        }
        if (PyList_SetItem(list, j, node) < 0)    // steals ref
        {
            Py_DECREF(node);
            Py_DECREF(list);
            return nullptr;
        }
    }
    return list;
}

bool ShapeFeatureBuilder::fromLineString(const GEOSGeometry* linestring)
{
    /*
    if (feature_->type != PyChangedFeature::WAY)
    {
        if (feature_->type != PyChangedFeature::UNASSIGNED)
        {
            PyErr_SetString(PyExc_ValueError,
                            "LineString can only be set for way");
            return false;
        }
        feature_->type = PyChangedFeature::WAY;
    }
     */

    const GEOSCoordSequence* coords = GEOSGeom_getCoordSeq_r(context_, linestring);
    unsigned int count = 0;
    GEOSCoordSeq_getSize_r(context_, coords, &count);

    // special‐case very large LineStrings in the future
    if (count > MAX_WAY_NODES)   [[unlikely]]
    {
        // TODO
        return true;
    }

    PyObject* list = nodesFromCoords(coords, 0, count);
    if (!list) return false;

    // TODO: clear old coords, set

    return true;
}

// TODO: minimum node count per segment
bool ShapeFeatureBuilder::createLineStringParts(PyObject* list,
    const GEOSGeometry* ring, PyObject* roleString)
{
    const GEOSCoordSequence* coords = GEOSGeom_getCoordSeq_r(context_, ring);
    unsigned int nodeCount = 0;
    GEOSCoordSeq_getSize_r(context_, coords, &nodeCount);
    unsigned int partCount = ((nodeCount-1) / MAX_WAY_NODES) + 1;
    unsigned int nodesPerPart = nodeCount / partCount + 2;
    unsigned int start = 0;
    for (unsigned int i=0; i<partCount; i++)
    {
        unsigned int end = std::min(start+nodesPerPart, nodeCount);
        PyObject* nodes = nodesFromCoords(coords, start, end);
        if (!nodes) return false;
        PyChangedFeature* way = changes_->createWay(nodes);
        if (!way) return false;
        PyChangedFeature* member = PyChangedFeature::createMember(way, roleString);
        if (!member)
        {
            Py_DECREF(way);
            return false;
        }
        if (PyList_Append(list, member) < 0)
        {
            Py_DECREF(member);
            return false;
        }
        start += nodesPerPart - 1;
    }
    return true;
}

bool ShapeFeatureBuilder::createPolygonParts(PyObject* list, const GEOSGeometry* polygon)
{
    const GEOSGeometry* outerRing = GEOSGetExteriorRing_r(context_, polygon);
    int holeCount = GEOSGetNumInteriorRings_r(context_, polygon);
    if (!createLineStringParts(list, outerRing, changes_->outerString))
    {
        return false;
    }
    for (int i = 0; i < holeCount; i++)
    {
        const GEOSGeometry* innerRing = GEOSGetInteriorRingN_r(context_, polygon, i);
        if (!createLineStringParts(list, innerRing, changes_->innerString))
        {
            return false;
        }
    }
    return true;
}

bool ShapeFeatureBuilder::fromPolygon(const GEOSGeometry* polygon)
{
    const GEOSGeometry* outerRing = GEOSGetExteriorRing_r(context_, polygon);
    const GEOSCoordSequence* coords = GEOSGeom_getCoordSeq_r(context_, outerRing);
    unsigned int nodeCount = 0;
    GEOSCoordSeq_getSize_r(context_, coords, &nodeCount);
    int holeCount = GEOSGetNumInteriorRings_r(context_, polygon);
    if (holeCount > 0 || nodeCount > MAX_WAY_NODES) [[unlikely]]
    {
        // must create polygon as a multi-polygon relation

        // TODO: ensure feature is relation or unassigned

        PyObject* list = PyList_New(0);
        if (!createPolygonParts(list, polygon))
        {
            Py_DECREF(list);
            return false;
        }

        // TODO: set members

        return true;
    }

    PyObject* list = nodesFromCoords(coords, 0, nodeCount);
    if (!list) return false;

    // TODO: clear old coords, set

    return true;
}

bool ShapeFeatureBuilder::fromMultiPolygon(const GEOSGeometry* multiPolygon)
{
    PyObject* list = PyList_New(0);
    int count = GEOSGetNumGeometries_r(context_, multiPolygon);
    for (int i = 0; i < count; i++)
    {
        const GEOSGeometry* polygon = GEOSGetGeometryN_r(context_, multiPolygon, i);
        if (!createPolygonParts(list, polygon))
        {
            Py_DECREF(list);
            return false;
        }
    }

    // TODO: set

    return true;
}

bool ShapeFeatureBuilder::fromGeometryCollection(const GEOSGeometry* geom)
{
    return true;
}


/*
bool PyChangedFeature::setLineString(GEOSContextHandle_t context, GEOSGeometry* geom)
{
    // TODO
    return true;
}

bool PyChangedFeature::setPolygon(GEOSContextHandle_t context, GEOSGeometry* geom)
{
    // TODO
    return true;
}

*/