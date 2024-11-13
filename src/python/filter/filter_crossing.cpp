// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "filters.h"
#include <geodesk/filter/CrossesFilter.h>
#include "python/feature/PyFeature.h"

PyFeatures* filters::crossing(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	CrossesFilterFactory factory;
	return filter(self, args, kwargs, factory);
}
