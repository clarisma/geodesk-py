// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyTagUtils.h"
#include <geodesk/feature/TagWalker.h>
#include "python/feature/PyFeature.h"

using namespace clarisma;

static PyObject* getKey(const TagWalker& iter)
{
	const int keyCode = iter.keyCode();
	if (keyCode >= 0) return iter.strings().getStringObject(keyCode);
	return Python::toStringObject(iter.key());
}

/// @brief Creates a dict from the tags of the given feature.
///
/// @return The key-value dict, or NULL if unable to allocate
///
PyObject* PyTagUtils::createTags(TagTablePtr pTags, StringTable& strings)
{
	PyObject* dict = PyDict_New();
	if (!dict) return nullptr;
	if (loadTags(dict, pTags, strings) < 0)
	{
		Py_DECREF(dict);
		return nullptr;
	}
	return dict;
}

int PyTagUtils::loadTags(PyObject* dict, TagTablePtr pTags, StringTable& strings)
{
	TagWalker iter(pTags, strings);
	while (iter.next())
	{
		PyObject* key = getKey(iter);
		if (!key) return -1;
		PyObject* value = iter.tags().valueAsObject(iter.rawValue(), strings);
		if (!value)
		{
			Py_DECREF(key);
			return -1;
		}

		if (PyDict_SetItem(dict, key, value) < 0)
		{
			Py_DECREF(key);
			Py_DECREF(value);
			return -1;
		}
		Py_DECREF(key);
		Py_DECREF(value);
	}
	return 0;
}

bool PyTagUtils::isAtomicTagValue(PyObject* obj)
{
	return PyUnicode_Check(obj) || PyLong_Check(obj) ||
		PyFloat_Check(obj) || PyBool_Check(obj);
}

bool PyTagUtils::isTagValue(PyObject* obj)
{
	if (isAtomicTagValue(obj)) return true;
	if (!PyList_CheckExact(obj) && !PyTuple_CheckExact(obj))
	{
		Py_ssize_t size = PySequence_Size(obj);
		for (Py_ssize_t i = 0; i < size; ++i)
		{
			PyObject* item = PySequence_GetItem(obj, i);
			if (!item)
			{
				PyErr_Clear();  // Ignore sequence access error
				return false;
			}
			bool valid = isAtomicTagValue(item);
			Py_DECREF(item);
			if (!valid)	return false;
		}
		return true;
	}
	return false;
}

int PyTagUtils::setTag(PyObject* tags, PyObject* key, PyObject* value)
{
	if (value == Py_None ||
		(PyUnicode_Check(value) && PyUnicode_GetLength(value) == 0))
	{
		if (PyDict_DelItem(tags, key) < 0)
		{
			if (!PyErr_ExceptionMatches(PyExc_KeyError))  // true error
			{
				return -1;
			}
			PyErr_Clear(); // ignore missing keys
		}
		return 0;
	}
	if (!isTagValue(value))
	{
		PyErr_SetString(PyExc_TypeError,
			"Tag value must be string, number, bool or list");
		return -1;
	}
	return PyDict_SetItem(tags, key, value);
}

int PyTagUtils::merge(PyObject* destTags, PyObject* srcTags)
{
	assert(PyDict_Check(destTags));
	assert(PyDict_Check(srcTags));
	PyObject* key;
	PyObject* value;
	Py_ssize_t pos = 0;

	while (PyDict_Next(srcTags, &pos, &key, &value))
	{
		if (setTag(destTags, key, value) < 0) return -1;
	}
	return 0;
}

int PyTagUtils::equals(PyObject* dict, TagTablePtr tags, StringTable& strings)
{
	if (PyDict_GET_SIZE(dict) != tags.count()) return 0;
	TagWalker iter(tags, strings);
	while (iter.next())
	{
		PyObject* key = getKey(iter);
		if (!key) return -1;
		PyObject *value = PyDict_GetItem(dict,key);		// borrowed ref
		if (!value) return 0;
		if (PyUnicode_Check(value))
		{
			std::string_view strValue = Python::stringAsStringView(value);
				// TODO: could fail
			if (iter.isStringValue())	[[likely]]
			{
				if (strValue !=	*iter.stringValueFast())
				{
					return 0;
				}
			}
			else
			{
				// compare string to number
				Decimal d(strValue, true);
				if (d != iter.numberValueFast()) return 0;
			}
		}
		else if (iter.isNumericValue())
		{
			PyObject* numObj = PyNumber_Float(value);
			if (!numObj)
			{
				PyErr_Clear();
				return 0;
			}
			double v = PyFloat_AS_DOUBLE(numObj);
			Py_DECREF(numObj);
			if (v != iter.numberValueFast()) return 0;
		}
		else if (PyBool_Check(value))
		{

		}

	}
}


void PyTagUtils::write(Buffer& out, PyObject* value)
{
	if (PyUnicode_Check(value))
	{
		out << Python::stringAsStringView(value);
		// TODO: could fail
		return;
	}
	if (PyBool_Check(value))
	{
		const std::string_view s = (value == Py_True) ? "yes" : "no";
		out << s;
		return;
	}
	PyObject* seq = PySequence_Fast(obj,
		"Tag value must be str, int, float, bool or list/tuple");
	if (!seq) return -1;
	if (!PyList_Check(value) && !PyTuple_Check(value))
	{

	}
	return PyUnicode_Check(obj) || PyLong_Check(obj) ||
	PyFloat_Check(obj) ||

	else if (iter.isNumericValue())
	{
		PyObject* numObj = PyNumber_Float(value);
		if (!numObj)
		{
			PyErr_Clear();
			return 0;
		}
		double v = PyFloat_AS_DOUBLE(numObj);
		Py_DECREF(numObj);
		if (v != iter.numberValueFast()) return 0;
	}
	else if (PyBool_Check(value))
	{

	}

}
