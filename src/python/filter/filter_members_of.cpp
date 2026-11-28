// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "filters.h"
#include "python/feature/PyFeature.h"


PyFeatures* filters::members_of(PyFeatures* self, PyObject* args, PyObject* kwargs)
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
			"members_of is not implemented for this type of feature set");
		return NULL;
	}

	if (arg->ob_type == &PyFeature::TYPE)
	{
		PyFeature* feature = (PyFeature*)arg;
		if (feature->feature.isRelation())
		{
			return self->withMembersOf(RelationPtr(feature->feature));
		}
		if (feature->feature.isWay())
		{
			// TODO: Should this only return feature nodes?
			return self->withNodesOf(WayPtr(feature->feature));
		}
		return PyFeatures::getEmpty();
	}
	if (arg->ob_type == &PyAnonymousNode::TYPE)
	{
		return PyFeatures::getEmpty();
	}

	PyErr_Format(PyExc_TypeError, "Expected Feature (instead of %s)", arg->ob_type->tp_name);
	return NULL;
}
