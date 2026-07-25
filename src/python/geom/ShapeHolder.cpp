// Copyright (c) 2025 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "ShapeHolder.h"
#include <geodesk/geom/geos/Geos.h>
#include "python/Environment.h"
#include "python/feature/PyFeature.h"
#include "python/geom/PyBox.h"
#include "python/geom/PyCoordinate.h"

using namespace clarisma;
using namespace geodesk;

bool ShapeHolder::setFromObject(PyObject* obj, GEOSContextHandle_t ctx)
{
    if (geom_.flags())
    {
        GEOSGeom_destroy_r(context_, geom_.ptr());
        geom_ = TaggedPtr<GEOSGeometry,1>();
    }
    context_ = ctx;
    coord_ = Coordinate();

    GEOSGeometry* geom = nullptr;
    PyTypeObject* type = Py_TYPE(obj);
    if (type == &PyCoordinate::TYPE)
    {
        PyCoordinate* c = static_cast<PyCoordinate*>(obj);
        coord_.x = c->x;
        coord_.y = c->y;
        return true;
    }
    if (type == &PyFeature::TYPE)
    {
        PyFeature* f = static_cast<PyFeature*>(obj);
        FeaturePtr feature = f->feature;
        coord_.x = Math::avg(feature.minX(),feature.maxX());
        coord_.y = Math::avg(feature.minY(),feature.maxY());
        if (feature.isNode())
        {
            NodePtr node(feature);
            coord_ = node.xy();
            return true;
        }
        if (feature.isWay())
        {
            geom = GeometryBuilder::buildWayGeometry(WayPtr(feature), ctx);
        }
        else
        {
            geom = GeometryBuilder::buildRelationGeometry(
                f->store, RelationPtr(feature), ctx);
        }
    }
    else if (type == &PyAnonymousNode::TYPE)
    {
        PyAnonymousNode* n = static_cast<PyAnonymousNode*>(obj);
        coord_.x = n->x_;
        coord_.y = n->y_;
        return true;
    }
    else if (type == &PyBox::TYPE)
    {
        PyBox* b = static_cast<PyBox*>(obj);
        coord_ = b->box.center();
        geom = GeometryBuilder::buildBoxGeometry(b->box, ctx);
    }
    else if (Environment::get().getGeosGeometry(obj, &geom))
    {
        geom_ = TaggedPtr<GEOSGeometry,1>(geom, 0);
        return true;
    }
    else
    {
        PyErr_Format(PyExc_TypeError,
            "Expected Geometry, Feature, Coordinate or Box (instead of %s)",
                    type->tp_name);
        return false;
    }
    if (!geom)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to build GEOS Geometry");
        return false;
    }
    geom_ = TaggedPtr<GEOSGeometry,1>(geom, 1);
    return true;
}


GEOSGeometry* ShapeHolder::asGeometry()
{
    if (isCoordinate())
    {
        GEOSGeometry* geom = geodesk::GeometryBuilder::buildPointGeometry(
            coord_.x, coord_.y, context_);
        geom_ = TaggedPtr<GEOSGeometry,1>(geom, geom != nullptr);
    }
    return geom_.ptr();
}


Coordinate ShapeHolder::asCoordinate()
{
    if (coord_.isNull())
    {
        coord_ = Geos::getEnvelope(context_, geom_.ptr()).center();
    }
    return coord_;
}