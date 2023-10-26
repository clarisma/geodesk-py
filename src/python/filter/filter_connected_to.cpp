// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "filters.h"
#include "filter/ConnectedFilter.h"
#include "python/feature/PyFeature.h"
#include "python/util/util.h"


PyFeatures* filters::connected_to(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	PyObject* arg = Python::checkSingleArg(args, kwargs, "feature");
	if (arg == NULL) return NULL;
	arg = Python::checkType(arg, &PyFeature::TYPE);
	if (arg == NULL) return NULL;
	PyFeature* feature = (PyFeature*)arg;
	return self->withFilter(new ConnectedFilter(feature->store, feature->feature));
}