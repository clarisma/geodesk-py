// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyChangedFeature.h"

#include <clarisma/util/Buffer.h>
#include <clarisma/util/DynamicStackBuffer.h>
#include <geodesk/feature/FeatureUtils.h>
#include <geodesk/feature/NodePtr.h>
#include <python/geom/PyMercator.h>

#include "python/feature/PyFeature.h"
#include "python/geom/PyCoordinate.h"
#include "PyChangedMembers.h"
#include "PyChanges.h"


bool PyChangedFeature::setMembers(PyObject* value)
{
	// TODO
	return false;
}

bool PyChangedFeature::setNodes(PyObject* value)
{
	// TODO
	return false;
}

bool PyChangedFeature::setShape(PyObject* value)
{
	// TODO
	return false;
}


bool PyChangedFeature::setShape(GEOSContextHandle_t context, GEOSGeometry* geom)
{
	switch (GEOSGeomTypeId_r(context, geom))
	{
	case GEOS_POINT:
		return setPoint(context, geom);
	case GEOS_LINESTRING:
	case GEOS_LINEARRING:
		return setLineString(context, geom);
	case GEOS_POLYGON:
		return setPolygon(context, geom);
	case GEOS_MULTIPOLYGON:
		return setMultiPolygon(context, geom);
	case GEOS_GEOMETRYCOLLECTION:
		return setGeometryCollection(context, geom);
	default:
		PyErr_SetString(PyExc_ValueError, "Unsupported geometry type");
		return false;
	}
}

bool PyChangedFeature::setPoint(GEOSContextHandle_t context, GEOSGeometry* geom)
{
	if (type != NODE)
	{
		if (type != UNASSIGNED)
		{
			PyErr_SetString(PyExc_ValueError, "Point can only be set for node");
			return false;
		}
		type = NODE;
	}
	const GEOSCoordSequence* coords = GEOSGeom_getCoordSeq_r(context, geom);
	double xOrLon = 0;
	double yOrLat = 0;
	GEOSCoordSeq_getXY_r(context, coords, 0, &xOrLon, &yOrLat);
	Coordinate xy = PyMercator::getAgnosticCoordinate(xOrLon,yOrLat);
	x = xy.x;
	y = xy.y;
	return true;
}

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

bool PyChangedFeature::setMultiPolygon(GEOSContextHandle_t context, GEOSGeometry* geom)
{
	// TODO
	return true;
}

bool PyChangedFeature::setGeometryCollection(GEOSContextHandle_t context, GEOSGeometry* geom)
{
	// TODO
	return true;
}

