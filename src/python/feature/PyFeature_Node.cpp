// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyFeature.h"

#include <geodesk/geom/GeometryBuilder.h>
#include <geodesk/geom/Mercator.h>
#include "python/Environment.h"
#include "python/format/PyFormatter.h"
#include "python/geom/PyBox.h"
#include "python/geom/PyCoordinate.h"
#include "python/query/PyFeatures.h"
#include "python/util/util.h"

using namespace clarisma;

PyObject* PyFeature::Node::bounds(PyFeature* self)
{
    DataPtr p = self->feature.ptr();
    int32_t x = (p-8).getInt();
    int32_t y = (p-4).getInt();
    return PyBox::create(x, y, x, y);
}

PyObject* PyFeature::Node::centroid(PyFeature* self)
{
    DataPtr p = self->feature.ptr();
    int32_t x = (p-8).getInt();
    int32_t y = (p-4).getInt();
    return PyCoordinate::create(x, y);
}

PyObject* PyFeature::Node::lat(PyFeature* self)
{
    DataPtr p = self->feature.ptr();
    return PyCoordinate::niceLatFromY((p-4).getInt());
}

PyObject* PyFeature::Node::lon(PyFeature* self)
{
    DataPtr p = self->feature.ptr();
    return PyCoordinate::niceLonFromX((p-8).getInt());
}

PyObject* PyFeature::Node::parents(PyFeature* self)
{
    FeatureTypes types = self->feature.isRelationMember() ?
        FeatureTypes::RELATIONS : 0;
    types |= (self->feature.flags() & FeatureFlags::WAYNODE) ?
        (FeatureTypes::WAYS & FeatureTypes::WAYNODE_FLAGGED) : 0;
    if (types == 0) return PyFeatures::getEmpty();
    return PyFeatures::parentsOf(self->store, self->feature, types);
}

PyObject* PyFeature::Node::shape(PyFeature* self)
{
    Environment& env = Environment::get();
    GEOSContextHandle_t geosContext = env.getGeosContext();
    if (!geosContext) return NULL;
    GEOSGeometry* geom = GeometryBuilder::buildNodeGeometry(NodePtr(self->feature), geosContext);
    return env.buildShapelyGeometry(geom);
}


PyObject* PyFeature::Node::x(PyFeature* self)
{
    DataPtr p = self->feature.ptr();
    return PyLong_FromLong((p-8).getInt());
}

PyObject* PyFeature::Node::y(PyFeature* self)
{
    DataPtr p = self->feature.ptr();
    return PyLong_FromLong((p-4).getInt());
}


AttrFunctionPtr const PyFeature::Node::FEATURE_METHODS[] =
{
    return_zero,        // area
    bounds,             // bounds
    buffer_method,      // buffer
    centroid,           // centroid
    distance_method,    // distance
    (AttrFunctionPtr)PyFormatter::geojson,   // geojson
    id,                 // id
    return_false,       // is_area
    return_true,        // is_node
    return_false,       // is_placeholder
    return_false,       // is_relation
    return_false,       // is_way
    lat,                // lat
    return_zero,        // length
    lon,                // lon
    map,                // map
    return_empty,       // members
    return_empty,       // nodes
    num_method,         // num
    osm_type,           // osm_type
    parents,            // parents
    role,               // role
    shape,              // shape
    str_method,         // str
    tags,               // tags
    (AttrFunctionPtr)PyFormatter::wkt,   // wkt
    x,                  // x
    y,                  // y
};
