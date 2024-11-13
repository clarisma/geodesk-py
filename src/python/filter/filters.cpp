// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "filters.h"
#include <geodesk/filter/Filter.h>
#include "python/Environment.h"
#include "python/feature/PyFeature.h"
#include "python/geom/PyBox.h"
#include "python/geom/PyCoordinate.h"
#include "python/util/util.h"


PyFeatures* filters::ancestors_of(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	PyErr_SetString(PyExc_NotImplementedError,
		"ancestors_of has not been implemented yet");
	return NULL;
}

PyFeatures* filters::contained_by(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	PyErr_SetString(PyExc_NotImplementedError,
		"contained_by has not been implemented yet");
	return NULL;
}

PyFeatures* filters::descendants_of(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	PyErr_SetString(PyExc_NotImplementedError,
		"descendants_of has not been implemented yet");
	return NULL;
}

PyFeatures* filters::disjoint_from(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	PyErr_SetString(PyExc_NotImplementedError,
		"disjoint_from has not been implemented yet");
	return NULL;
}

PyFeatures* filters::max_meters_from(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	PyErr_SetString(PyExc_NotImplementedError,
		"max_meters_from has not been implemented yet");
	return NULL;
}

PyFeatures* filters::nearest_to(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	PyErr_SetString(PyExc_NotImplementedError,
		"nearest_to has not been implemented yet");
	return NULL;
}

PyFeatures* filters::overlapping(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	PyErr_SetString(PyExc_NotImplementedError,
		"overlapping has not been implemented yet");
	return NULL;
}

PyFeatures* filters::pythonFilter(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	PyErr_SetString(PyExc_NotImplementedError,
		"filter has not been implemented yet");
	return NULL;
}

PyFeatures* filters::touching(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	PyErr_SetString(PyExc_NotImplementedError,
		"touching has not been implemented yet");
	return NULL;
}

PyFeatures* filters::with_role(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	PyErr_SetString(PyExc_NotImplementedError,
		"with_role has not been implemented yet");
	return NULL;
}


PyFeatures* filters::filter(PyFeatures* self, PyObject* args, PyObject* kwargs, PreparedFilterFactory& factory)
{
	PyObject* arg = Python::checkSingleArg(args, kwargs, "geom");
	if (arg == NULL) return NULL;
	PyTypeObject* type = Py_TYPE(arg);
	GEOSGeometry* geom;
	const Filter* filter = nullptr;

	if (type == &PyFeature::TYPE)
	{
		PyFeature* feature = (PyFeature*)arg;
		filter = factory.forFeature(feature->store, feature->feature);
	}
	else if (Environment::get().getGeosGeometry(arg, &geom))
	{
		GEOSContextHandle_t context = Environment::get().getGeosContext();
		// TODO: This may return with an exception set if GEOS library
		// cannot be initialized
		filter = factory.forGeometry(context, geom);
	}
	else if (type == &PyBox::TYPE)
	{
		PyBox* box = (PyBox*)arg;
		filter = factory.forBox(box->box);
	}
	else if (type == &PyCoordinate::TYPE)
	{
		PyCoordinate* coord = (PyCoordinate*)arg;
		filter = factory.forCoordinate(Coordinate(coord->x, coord->y));
	}
	else if (type == &PyAnonymousNode::TYPE)
	{
		PyAnonymousNode* node = (PyAnonymousNode*)arg;
		filter = factory.forCoordinate(Coordinate(node->x_, node->y_));
	}
	else
	{
		PyErr_Format(PyExc_TypeError, "Expected geometric object instead of %s", type->tp_name);
		return NULL;
	}

	if (filter) return self->withFilter(filter);
	return self->getEmpty();
}

