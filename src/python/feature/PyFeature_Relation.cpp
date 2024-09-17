// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyFeature.h"
#include "geom/GeometryBuilder.h"
#include "geom/polygon/Polygonizer.h"
#include "geom/Area.h"
#include "geom/Centroid.h"
#include "geom/Length.h"
#include "python/Environment.h"
#include "python/format/PyFormatter.h"
#include "python/geom/PyCoordinate.h"
#include "python/query/PyFeatures.h"
#include "python/util/util.h"


PyObject* PyFeature::Relation::area(PyFeature* self)
{
    RelationPtr relation(self->feature);
    if (!relation.isArea()) return PyLong_FromLong(0);
        // TODO: for non-area realtions, should we return the 
        // total area of members that are areas?
    return PyFloat_FromDouble(Area::ofRelation(self->store, relation));
}


PyObject* PyFeature::Relation::centroid(PyFeature* self)
{
    return PyCoordinate::create(Centroid::ofRelation(self->store, RelationPtr(self->feature)));
}

PyObject* PyFeature::Relation::is_placeholder(PyFeature* self)
{
    return Python::boolValue(RelationPtr(self->feature).isPlaceholder());
}


PyObject* PyFeature::Relation::length(PyFeature* self)
{
    return PyFloat_FromDouble(Length::ofRelation(self->store, RelationPtr(self->feature)));
}


PyObject* PyFeature::Relation::members(PyFeature* self)
{
    DataPtr pBody = self->feature.bodyptr();
    if (pBody.getInt() == 0)
    {
        // return (PyObject*)Environment::get().getEmptyFeatures();
        return self->store->getEmptyFeatures();
    }
    return PyFeatures::create(&PyFeatures::Members::SUBTYPE, 
        self->store, self->feature, FeatureTypes::RELATION_MEMBERS);
}

PyObject* PyFeature::Relation::shape(PyFeature* self)
{
    Environment& env = Environment::get();
    GEOSContextHandle_t geosContext = env.getGeosContext();
    if (!geosContext) return NULL;

    RelationPtr rel(self->feature);
    /*
    if (rel.isArea())
    {
        Polygonizer polygonizer;
        polygonizer.createRings(self->store, rel);
        polygonizer.assignAndMergeHoles();
        return env.buildShapelyGeometry(polygonizer.createPolygonal(geosContext));
    }
    // TODO: other geometry collections
    */

    return env.buildShapelyGeometry(GeometryBuilder::buildRelationGeometry(
        self->store, rel, geosContext));
}

AttrFunctionPtr const PyFeature::Relation::FEATURE_METHODS[] =
{
    area,               // area
    bounds,             // bounds
    centroid,           // centroid
    (AttrFunctionPtr)PyFormatter::geojson,   // geojson
    id,                 // id
    is_area,            // is_area
    return_false,       // is_node
    is_placeholder,     // is_placeholder
    return_true,        // is_relation
    return_false,       // is_way
    lat,                // lat
    length,             // length
    lon,                // lon
    map,                // map
    members,            // members
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
