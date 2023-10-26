// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include "filter/PreparedFilterFactory.h"
#include "python/query/PyFeatures.h"

namespace filters
{
	extern PyFeatures* filter(PyFeatures* self, PyObject* args, PyObject* kwargs, PreparedFilterFactory& factory);
	extern PyFeatures* ancestors_of(PyFeatures* self, PyObject* args, PyObject* kwargs);
	extern PyFeatures* around(PyFeatures* self, PyObject* args, PyObject* kwargs);
	extern PyFeatures* connected_to(PyFeatures* self, PyObject* args, PyObject* kwargs);
	extern PyFeatures* contains(PyFeatures* self, PyObject* args, PyObject* kwargs);
	extern PyFeatures* contained_by(PyFeatures* self, PyObject* args, PyObject* kwargs);
	extern PyFeatures* descendants_of(PyFeatures* self, PyObject* args, PyObject* kwargs);
	extern PyFeatures* disjoint(PyFeatures* self, PyObject* args, PyObject* kwargs);
	extern PyFeatures* intersects(PyFeatures* self, PyObject* args, PyObject* kwargs);
	extern PyFeatures* max_area(PyFeatures* self, PyObject* args, PyObject* kwargs);
	extern PyFeatures* max_length(PyFeatures* self, PyObject* args, PyObject* kwargs);
	extern PyFeatures* max_meters_from(PyFeatures* self, PyObject* args, PyObject* kwargs);
	extern PyFeatures* min_area(PyFeatures* self, PyObject* args, PyObject* kwargs);
	extern PyFeatures* members_of(PyFeatures* self, PyObject* args, PyObject* kwargs);
	extern PyFeatures* min_length(PyFeatures* self, PyObject* args, PyObject* kwargs);
	extern PyFeatures* nearest_to(PyFeatures* self, PyObject* args, PyObject* kwargs);
	extern PyFeatures* nodes_of(PyFeatures* self, PyObject* args, PyObject* kwargs);
	extern PyFeatures* overlaps(PyFeatures* self, PyObject* args, PyObject* kwargs);
	extern PyFeatures* parents_of(PyFeatures* self, PyObject* args, PyObject* kwargs);
	extern PyFeatures* touches(PyFeatures* self, PyObject* args, PyObject* kwargs);
	extern PyFeatures* with_role(PyFeatures* self, PyObject* args, PyObject* kwargs);
	extern PyFeatures* within(PyFeatures* self, PyObject* args, PyObject* kwargs);
}

