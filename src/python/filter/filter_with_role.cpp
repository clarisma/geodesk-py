// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "filters.h"
#include <clarisma/data/SmallVector.h>
#include <geodesk/filter/RoleFilter.h>

using namespace clarisma;


PyFeatures* filters::with_role(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	size_t argCount = PyTuple_GET_SIZE(args);

	if (argCount==0 || (kwargs && PyDict_GET_SIZE(kwargs) > 0))
	{
		PyErr_SetString(PyExc_TypeError,
			"with_role: expected one or more role strings");
		return NULL;
	}

	if (argCount==1)
	{
		PyObject* arg = PyTuple_GET_ITEM(args, 0);
		if (!PyUnicode_Check(arg)) args = arg;
	}
	PyObject* seq = PySequence_Fast(args, "expected an iterable");
	if (!seq) return NULL;

	SmallVector<std::string_view,8> roles;

	Py_ssize_t size = PySequence_Fast_GET_SIZE(seq);
	for (Py_ssize_t i = 0; i < size; ++i)
	{
		Py_ssize_t strLen;
		PyObject* item = PySequence_Fast_GET_ITEM(seq, i);
		const char* s = PyUnicode_AsUTF8AndSize(item, &strLen);
		if (!s)
		{
			Py_DECREF(seq);
			return NULL;
		}
		roles.push_back(std::string_view(s, strLen));
	}

	// Passing an empty iterable is not an error; it just means
	// that the RoleFilter will not accept any members

	const RoleFilter* filter = new RoleFilter(roles, self->store->strings());
	Py_DECREF(seq);
		// Only release sequence after RoleFilter has copied the strings
		// (If the sequence is generated, the temporary string objects
		// may otherwise be destroyed prematurely)
	return self->withFilter(filter);
}
