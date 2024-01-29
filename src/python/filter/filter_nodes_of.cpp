// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "filters.h"
#include "python/feature/PyFeature.h"

PyFeatures* filters::nodes_of(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	PyObject* arg = Python::checkSingleArg(args, kwargs, "Feature");
	if (!arg) return NULL;

	if (self->selectionType != &PyFeatures::World::SUBTYPE)
	{
		PyErr_SetString(PyExc_NotImplementedError,
			"nodes_of is not implemented for this type of feature set");
		return NULL;
	}

	if (arg->ob_type == &PyFeature::TYPE)
	{
		PyFeature* feature = (PyFeature*)arg;
		// TODO: What does "nodes" return for a relation? (Right now, an empty set)
		if (feature->feature.isWay())
		{
			return PyFeatures::createRelated(self, &PyFeatures::WayNodes::SUBTYPE,
				feature->feature, FeatureTypes::NODES & FeatureTypes::WAYNODE_FLAGGED);
		}
		return Environment::get().getEmptyFeatures();
	}
	if (arg->ob_type == &PyAnonymousNode::TYPE)
	{
		return PyFeatures::getEmpty();
	}

	PyErr_Format(PyExc_TypeError, "Expected Feature (instead of %s)", arg->ob_type->tp_name);
	return NULL;
}
