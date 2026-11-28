// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "filters.h"
#include "python/feature/PyFeature.h"


PyFeatures* filters::parents_of(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	PyObject* arg = (PyFeature*)Python::checkSingleArg(args, kwargs, "Feature");
	if (arg == NULL) return NULL;

	if (self->kind() != Selection::WORLD)
	{
		if (self->kind() == Selection::EMPTY)
		{
			return Python::newRef(self);
		}
		PyErr_SetString(PyExc_NotImplementedError,
			"parents_of is not implemented for this type of feature set");
		return NULL;
	}

	if (arg->ob_type == &PyFeature::TYPE)
	{
		PyFeature* feature = (PyFeature*)arg;
		return self->withParentsOf(feature->feature);
	}
	if (arg->ob_type == &PyAnonymousNode::TYPE)
	{
		Coordinate xy(((PyAnonymousNode*)arg)->x_, ((PyAnonymousNode*)arg)->y_);
		return self->withParentWaysOf(xy);
		// Returns an empty set if this feature set does not include any ways
	}

	PyErr_Format(PyExc_TypeError, "Expected Feature (instead of %s)", arg->ob_type->tp_name);
	return NULL;
}
