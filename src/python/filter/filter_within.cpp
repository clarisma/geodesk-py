// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "filters.h"
#include <geodesk/filter/WithinFilter.h>
#include "python/feature/PyFeature.h"
#include "python/util/util.h"

PyFeatures* filters::within(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	WithinFilterFactory factory;
	return filter(self, args, kwargs, factory);
}
