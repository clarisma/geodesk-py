// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyChangedFeature.h"
#include <geodesk/feature/TagIterator.h>
#include "python/feature/PyFeature.h"


bool PyChangedFeature::setTags(PyObject* value)
{
	if (!Python::checkType(value, &PyDict_Type)) return false;
	if(tags_)
	{
		Py_DECREF(tags_);
		tags_ = nullptr;
	}
	return setOrRemoveTags(value);
}

/// @brief Creates a dict from the tags of the given feature.
///
/// @return The key-value dict, or NULL if unable to allocate
///
PyObject* PyChangedFeature::createTags(FeatureStore* store, FeaturePtr feature)
{
	PyObject* dict = PyDict_New();
	if (!dict) return nullptr;

	TagIterator iter(feature.tags(), store->strings());
	for (;;)
	{
		auto [keyStr, tagBits] = iter.next();
		if (keyStr == nullptr) break;

		PyObject* key = Python::toStringObject(keyStr->data(), keyStr->length());
		if (!key)
		{
			Py_DECREF(dict);
			return nullptr;
		}

		PyObject* value = iter.tags().valueAsObject(tagBits, store->strings());
		if (!value)
		{
			Py_DECREF(key);
			Py_DECREF(dict);
			return nullptr;
		}

		if (PyDict_SetItem(dict, key, value) < 0)
		{
			Py_DECREF(key);
			Py_DECREF(value);
			Py_DECREF(dict);
			return nullptr;
		}

		Py_DECREF(key);
		Py_DECREF(value);
	}
	return dict;
}

/// @brief Ensures the tags (as a Python dict) are loaded.
/// @param create If `true`, creates an empty dict if tags is NULL
/// @return 0 if tags remains NULL (new feature or anon node, and
///			create was set to false)
///			1 if tags is now a valid Python dict
///			-1 if dict could not be created
int PyChangedFeature::loadTags(bool create)
{
	assert(type_ != MEMBER);
	if (tags_) return 1;
	if (!original_ || Py_TYPE(original_) == &PyAnonymousNode::TYPE)
	{
		if (!create) return 0;
		tags_ = PyDict_New();
	}
	else
	{
		assert(Py_TYPE(original_) == &PyFeature::TYPE);
		PyFeature* feature = (PyFeature*)original_;
		tags_ = createTags(feature->store, feature->feature);
	}
	return tags_ ? 1 : -1;
}

bool PyChangedFeature::isAtomicTagValue(PyObject* obj)
{
	return PyUnicode_Check(obj) || PyLong_Check(obj) ||
		PyFloat_Check(obj) || PyBool_Check(obj);
}

bool PyChangedFeature::isTagValue(PyObject* obj)
{
	if (isAtomicTagValue(obj)) return true;
	if (!PyList_Check(obj) && !PyTuple_Check(obj))
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

bool PyChangedFeature::setOrRemoveTag(PyObject* key, PyObject* value)
{
	assert(type_ != MEMBER);
	if (loadTags(true) < 0) return false;
	if (value == Py_None ||
		(PyUnicode_Check(value) && PyUnicode_GetLength(value) == 0))
	{
		if (PyDict_DelItem(tags_, key) < 0)
		{
			if (!PyErr_ExceptionMatches(PyExc_KeyError))  // true error
			{
				return false;
			}
			PyErr_Clear(); // ignore missing keys
		}
		return true;
	}
	if (!isTagValue(value))
	{
		PyErr_SetString(PyExc_TypeError,
			"Tag value must be string, number, bool or list");
		return false;
	}
	return PyDict_SetItem(tags_, key, value) == 0;
}

bool PyChangedFeature::setOrRemoveTags(PyObject* dict)
{
	assert(PyDict_Check(dict));
	PyObject* key;
	PyObject* value;
	Py_ssize_t pos = 0;

	while (PyDict_Next(dict, &pos, &key, &value))
	{
		if (!setOrRemoveTag(key, value)) return false;
	}
	return true;
}

