// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "filters.h"
#include "python/feature/PyFeature.h"


PyFeatures* filters::members_of(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	PyObject* arg = (PyFeature*)Python::checkSingleArg(args, kwargs, "Feature");
	if (arg == NULL) return NULL;

	if (self->selectionType != &PyFeatures::World::SUBTYPE)
	{
		if (self->selectionType == &PyFeatures::Empty::SUBTYPE)
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
			return PyFeatures::createRelated(self, &PyFeatures::Members::SUBTYPE,
				feature->feature, FeatureTypes::ALL & FeatureTypes::RELATION_MEMBERS);
		}
		if (feature->feature.isWay())
		{
			// TODO: Should this only return feature nodes?
			return PyFeatures::createRelated(self, &PyFeatures::WayNodes::SUBTYPE,
				feature->feature, FeatureTypes::NODES & FeatureTypes::WAYNODE_FLAGGED);
		}
		return self->getEmpty();
	}
	if (arg->ob_type == &PyAnonymousNode::TYPE)
	{
		return self->getEmpty();
	}

	PyErr_Format(PyExc_TypeError, "Expected Feature (instead of %s)", arg->ob_type->tp_name);
	return NULL;
}
