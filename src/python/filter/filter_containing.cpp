// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "filters.h"
#include <geodesk/filter/ContainsPointFilter.h>
#include "python/feature/PyFeature.h"
#include "python/geom/PyCoordinate.h"
#include "python/util/util.h"

PyFeatures* filters::containing(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	PyObject* arg = Python::checkSingleArg(args, kwargs, "geom");
	if (arg == NULL) return NULL;
	PyTypeObject* type = Py_TYPE(arg);
	Filter* filter = nullptr;
	if (type == &PyFeature::TYPE)			// TODO: unify type system
	{
		PyFeature* feature = (PyFeature*)arg;
		if (feature->feature.isNode())
		{
			NodePtr node(feature->feature);
			filter = new ContainsPointFilter(node.xy());
		}
		else
		{
			// TODO
		}
	}
	else if (type == &PyCoordinate::TYPE)
	{
		PyCoordinate* coord = (PyCoordinate*)arg;
		filter = new ContainsPointFilter(Coordinate(coord->x, coord->y));
	}
	else
	{
		// TODO
	}

	if (filter) return self->withFilter(filter);

	PyErr_SetString(PyExc_NotImplementedError,
		"contains will be available in Version 0.2.0");
	return NULL;
}