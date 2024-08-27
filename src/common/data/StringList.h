// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <common/util/Strings.h>

template<typename T>
class StringList
{
public:
	std::string_view get()
};

#ifdef GEODESK_PYTHON
#include <Python.h>
#include "python/util/PythonException.h"	// TODO: move

template<>
class StringList<PyObject*>
{
public:
	StringList(PyObject* list) :
		list_(list)
	{
		assert(PyList_Check(list));
	}

	size_t size() const
	{
		return PyList_Size(list_);
	}


	std::string_view get(size_t n)
	{
		PyObject* item = PyList_GetItem(list_, n);
		if (item)
		{
			Py_ssize_t len;
			const char* s = PyUnicode_AsUTF8AndSize(item, &len);
			if (s) return std::string_view(s, len);
		}
		throw PythonException();
	}

private:
	PyObject* list_;
};

#endif
