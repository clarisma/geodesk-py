// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "filters.h"
#include "filter/Filter.h"
#include "python/feature/PyFeature.h"
#include "python/geom/PyBox.h"
#include "python/geom/PyCoordinate.h"
#include "python/util/util.h"


PyFeatures* filters::ancestors_of(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	PyErr_SetString(PyExc_NotImplementedError,
		"ancestors_of will be available in Version 0.2.0");
	return NULL;
}

PyFeatures* filters::contained_by(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	PyErr_SetString(PyExc_NotImplementedError,
		"contained_by will be available in Version 0.2.0");
	return NULL;
}

PyFeatures* filters::descendants_of(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	PyErr_SetString(PyExc_NotImplementedError,
		"descendants_of will be available in Version 0.2.0");
	return NULL;
}

PyFeatures* filters::disjoint(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	PyErr_SetString(PyExc_NotImplementedError,
		"disjoint will be available in Version 0.2.0");
	return NULL;
}

PyFeatures* filters::max_area(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	PyErr_SetString(PyExc_NotImplementedError,
		"max_area will be available in Version 0.2.0");
	return NULL;
}

PyFeatures* filters::max_length(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	PyErr_SetString(PyExc_NotImplementedError,
		"max_length will be available in Version 0.2.0");
	return NULL;
}

PyFeatures* filters::max_meters_from(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	PyErr_SetString(PyExc_NotImplementedError,
		"max_meters_from will be available in Version 0.2.0");
	return NULL;
}

PyFeatures* filters::min_area(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	PyErr_SetString(PyExc_NotImplementedError,
		"min_area will be available in Version 0.2.0");
	return NULL;
}


PyFeatures* filters::min_length(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	PyErr_SetString(PyExc_NotImplementedError,
		"min_length will be available in Version 0.2.0");
	return NULL;
}

PyFeatures* filters::nearest_to(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	PyErr_SetString(PyExc_NotImplementedError,
		"nearest_to will be available in Version 0.2.0");
	return NULL;
}

PyFeatures* filters::nodes_of(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	PyObject* arg = Python::checkSingleArg(args, kwargs, "Feature");
	if (!arg) return NULL;

	if (Py_TYPE(arg) != &PyFeature::TYPE)
	{
		if (Python::checkType(arg, &PyAnonymousNode::TYPE, "Feature") == NULL)
		{
			return NULL;
		}
		return PyFeatures::getEmpty();
	}

	PyFeature* feature = (PyFeature*)arg;
	if (self->selectionType == &PyFeatures::World::SUBTYPE)
	{
		return PyFeatures::createRelated(self, &PyFeatures::WayNodes::SUBTYPE,
			feature->feature, FeatureTypes::NODES);
	}
	PyErr_SetString(PyExc_NotImplementedError,
		"nodes_of is not implemented for this type of feature set");
	return NULL;
}

PyFeatures* filters::overlaps(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	PyErr_SetString(PyExc_NotImplementedError,
		"overlaps will be available in Version 0.2.0");
	return NULL;
}

PyFeatures* filters::parents_of(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	PyErr_SetString(PyExc_NotImplementedError,
		"parents_of will be available in Version 0.2.0");
	return NULL;
}

PyFeatures* filters::touches(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	PyErr_SetString(PyExc_NotImplementedError,
		"touches will be available in Version 0.2.0");
	return NULL;
}

PyFeatures* filters::with_role(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	PyErr_SetString(PyExc_NotImplementedError,
		"with_role will be available in Version 0.2.0");
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
	else
	{
		PyErr_Format(PyExc_TypeError, "Expected geometric object instead of %s", type->tp_name);
		return NULL;
	}

	if (filter) return self->withFilter(filter);
	return PyFeatures::getEmpty();
}

