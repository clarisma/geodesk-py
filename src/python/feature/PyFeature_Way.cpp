// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyFeature.h"
#include "feature/GeometryBuilder.h"
#include "geom/Area.h"
#include "geom/Centroid.h"
#include "geom/Length.h"
#include "geom/Mercator.h"
#include "python/Environment.h"
#include "python/format/PyFormatter.h"
#include "python/geom/PyCoordinate.h"
#include "python/query/PyFeatures.h"
#include "python/util/util.h"

/*
PyObject* PyWay::iter(PyFeature* self)
{
    return PyNodeIterator::create(self);
}
*/

PyObject* PyFeature::Way::area(PyFeature* self)
{
    WayRef way(self->feature);
    if (!way.isArea()) return PyLong_FromLong(0);
    return PyFloat_FromDouble(Area::ofWay(way));
}


PyObject* PyFeature::Way::centroid(PyFeature* self)
{
    return PyCoordinate::create(Centroid::ofWay(WayRef(self->feature)));
}

PyObject* PyFeature::Way::is_placeholder(PyFeature* self)
{
    return Python::boolValue(WayRef(self->feature).isPlaceholder());
}

PyObject* PyFeature::Way::length(PyFeature* self)
{
    return PyFloat_FromDouble(Length::ofWay(WayRef(self->feature)));
}

PyObject* PyFeature::Way::nodes(PyFeature* self)
{
    return PyFeatures::create(&PyFeatures::WayNodes::SUBTYPE,
        self->store, self->feature, FeatureTypes::NODES & FeatureTypes::WAYNODE_FLAGGED);
}

PyObject* PyFeature::Way::shape(PyFeature* self)
{
    Environment& env = Environment::get();
    GEOSContextHandle_t geosContext = env.getGeosContext();
    if (!geosContext) return NULL;
    GEOSGeometry* geom = GeometryBuilder::buildWayGeometry(self->feature, geosContext);
    return env.buildShapelyGeometry(geom);
}


AttrFunctionPtr const PyFeature::Way::FEATURE_METHODS[] =
{
    area,               // area
    bounds,             // bounds
    centroid,           // centroid
    (AttrFunctionPtr)PyFormatter::geojson, // geojson
    id,                 // id
    is_area,            // is_area
    return_false,       // is_node
    is_placeholder,     // is_placeholder
    return_false,       // is_relation
    return_true,        // is_way
    lat,                // lat
    length,             // length
    lon,                // lon
    map,                // map
    nodes,              // members
    nodes,              // nodes
    num_method,         // num
    osm_type,           // osm_type
    parents,            // parents
    role,               // role
    shape,              // shape
    str_method,         // str
    tags,               // tags
    (AttrFunctionPtr)PyFormatter::wkt, // wkt
    x,                  // x
    y,                  // y
};

