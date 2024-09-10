// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "filters.h"
#include "python/feature/PyFeature.h"


PyFeatures* filters::parents_of(PyFeatures* self, PyObject* args, PyObject* kwargs)
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
			"parents_of is not implemented for this type of feature set");
		return NULL;
	}

	if (arg->ob_type == &PyFeature::TYPE)
	{
		PyFeature* featureObj = (PyFeature*)arg;
		FeaturePtr feature = featureObj->feature;
		FeatureTypes acceptedTypes = feature.isRelationMember() ?
			FeatureTypes::RELATIONS : 0;
		acceptedTypes |= 
			(FeatureTypes(FeatureTypes::NODES & FeatureTypes::WAYNODE_FLAGGED).
				acceptFlags(feature.flags())) ? 
				(FeatureTypes::WAYS & FeatureTypes::WAYNODE_FLAGGED) : 0;
		return PyFeatures::createRelated(self, &PyFeatures::Parents::SUBTYPE,
			feature, acceptedTypes);
	}
	if (arg->ob_type == &PyAnonymousNode::TYPE)
	{
		return PyFeatures::Parents::create(self, (PyAnonymousNode*)arg);
		// Returns an empty set if this feature set does not include any ways
	}

	PyErr_Format(PyExc_TypeError, "Expected Feature (instead of %s)", arg->ob_type->tp_name);
	return NULL;
}
