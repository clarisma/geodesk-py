// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "filters.h"
#include <geodesk/filter/ContainsPointFilter.h>
#include <geodesk/geom/Centroid.h>
#include <geodesk/geom/geos/Geos.h>
#include "python/Environment.h"
#include "python/feature/PyFeature.h"
#include "python/geom/PyCoordinate.h"
#include "python/util/util.h"

PyFeatures* filters::containingPoint(PyFeatures* self, Coordinate xy)
{
	return self->withFilter(new ContainsPointFilter(xy));
}

PyFeatures* filters::containing(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	PyObject* arg = Python::checkSingleArg(args, kwargs, "geom");
	if (arg == NULL) return NULL;
	PyTypeObject* type = Py_TYPE(arg);
	Filter* filter = nullptr;
	if (type == &PyFeature::TYPE)			// TODO: unify type system
	{
		PyFeature* feature = (PyFeature*)arg;
		return containingPoint(self, Centroid::ofFeature(
			feature->store, feature->feature));
	}
	if (type == &PyCoordinate::TYPE)
	{
		PyCoordinate* coord = (PyCoordinate*)arg;
		return containingPoint(self, Coordinate(coord->x, coord->y));
	}
	if (type == &PyAnonymousNode::TYPE)
	{
		PyAnonymousNode* node = (PyAnonymousNode*)arg;
		return containingPoint(self, Coordinate(node->x_, node->y_));
	}

	GEOSGeometry* geom;
	if (Environment::get().getGeosGeometry(arg, &geom))
	{
		GEOSContextHandle_t context = Environment::get().getGeosContext();
		if (!context) return NULL;
		Coordinate centroid;
		if (!Geos::centroid(context, geom, &centroid))
		{
			return self->getEmpty();
		}
		return containingPoint(self, centroid);
	}

	PyErr_Format(PyExc_TypeError, "Expected geometric object instead of %s", type->tp_name);
	return NULL;
}