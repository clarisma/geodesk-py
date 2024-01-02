// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "filters.h"
#include "filter/IntersectsFilter.h"
#include "python/feature/PyFeature.h"
#include "python/util/util.h"

PyFeatures* filters::intersects(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	IntersectsFilterFactory factory;
	return filter(self, args, kwargs, factory);
}