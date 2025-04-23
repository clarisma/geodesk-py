// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyChangedFeature.h"
#include <geodesk/feature/TagIterator.h>
#include "python/feature/PyFeature.h"


bool PyChangedFeature::setTags(PyObject* value)
{
	if (!Python::checkType(value, &PyDict_Type)) return false;
	if(tags)
	{
		Py_DECREF(tags);
		tags = nullptr;
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
///			create was et to false)
///			1 if tags is now a valid Python dict
///			-1 if dict could not be created
int PyChangedFeature::loadTags(bool create)
{
	assert(type != MEMBER);
	if (tags) return 1;
	if (!original || Py_TYPE(original) == &PyAnonymousNode::TYPE)
	{
		if (!create) return 0;
		tags = PyDict_New();
	}
	else
	{
		assert(Py_TYPE(original) == &PyFeature::TYPE);
		PyFeature* feature = (PyFeature*)original;
		tags = createTags(feature->store, feature->feature);
	}
	return tags ? 1 : -1;
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
	assert(type != MEMBER);
	if (loadTags(true) < 0) return false;
	if (value == Py_None ||
		(PyUnicode_Check(value) && PyUnicode_GetLength(value) == 0))
	{
		if (PyDict_DelItem(tags, key) < 0)
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
	return PyDict_SetItem(tags, key, value) == 0;
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

PyObject* PyChangedFeature::getitem(PyChangedFeature* self, PyObject* key)
{
	if (self->type == MEMBER)	[[unlikely]]
	{
		self = self->member;	// delegate to member
	}
	int res = self->loadTags(false);
	if (res <= 0)
	{
		if (res < 0) return nullptr;
		Py_RETURN_NONE;
	}
	PyObject* value = PyDict_GetItem(self->tags, key);
	if (!value)
	{
		Py_RETURN_NONE;
	}
	return Python::newRef(value);
}

int PyChangedFeature::setitem(PyChangedFeature* self, PyObject* key, PyObject* value)
{
	if (self->type == MEMBER)	[[unlikely]]
	{
		self = self->member;	// delegate to member
	}
	int res = self->loadTags(true);
	if (res < 0) return -1;
	assert(res > 0);

	// value == nullptr => deletion
	// TODO: setting to empty string should also delete
	if (value == nullptr || value == Py_None)
	{
		return PyObject_DelItem(self->tags, key);
	}
	return PyObject_SetItem(self->tags, key, value);
}

void PyChangedFeature::format(clarisma::Buffer& buf)
{
	if (type == MEMBER)	[[unlikely]]
	{
		member->format(buf);
		buf << " as ";
		buf << Python::getStringView(role);
		return;
	}
	buf << typeName(static_cast<FeatureType>(type)) << '/' << id;
	// TODO: new, anonymous
}

bool PyChangedFeature::setShape(GEOSContextHandle_t context, GEOSGeometry* geom)
{
	switch (GEOSGeomTypeId_r(context, geom))
	{
	case GEOS_POINT:
		return setPoint(context, geom);
	case GEOS_LINESTRING:
	case GEOS_LINEARRING:
		return setLineString(context, geom);
	case GEOS_POLYGON:
		return setPolygon(context, geom);
	case GEOS_MULTIPOLYGON:
		return setMultiPolygon(context, geom);
	case GEOS_GEOMETRYCOLLECTION:
		return setGeometryCollection(context, geom);
	default:
		PyErr_SetString(PyExc_ValueError, "Unsupported geometry type");
		return false;
	}
}

bool PyChangedFeature::setPoint(GEOSContextHandle_t context, GEOSGeometry* geom)
{
	if (type != NODE)
	{
		if (type != UNASSIGNED)
		{
			PyErr_SetString(PyExc_ValueError, "Point can only be set for node");
			return false;
		}
		type = NODE;
	}
	const GEOSCoordSequence* coords = GEOSGeom_getCoordSeq_r(context, geom);
	double xOrLon = 0;
	double yOrLat = 0;
	GEOSCoordSeq_getXY_r(context, coords, 0, &xOrLon, &yOrLat);
	Coordinate xy = PyMercator::getAgnosticCoordinate(xOrLon,yOrLat);
	x = xy.x;
	y = xy.y;
	return true;
}

bool PyChangedFeature::setLineString(GEOSContextHandle_t context, GEOSGeometry* geom)
{
	// TODO
	return true;
}

bool PyChangedFeature::setPolygon(GEOSContextHandle_t context, GEOSGeometry* geom)
{
	// TODO
	return true;
}

bool PyChangedFeature::setMultiPolygon(GEOSContextHandle_t context, GEOSGeometry* geom)
{
	// TODO
	return true;
}

bool PyChangedFeature::setGeometryCollection(GEOSContextHandle_t context, GEOSGeometry* geom)
{
	// TODO
	return true;
}

bool PyChangedFeature::modify(PyObject* args, PyObject* kwargs)
{
	PyObject* list = nullptr;
	PyObject* key = nullptr;
	int seenArgs = 0;
	Coordinate xy;

	return true;	// TODO
}


PyMappingMethods PyChangedFeature::MAPPING_METHODS =
{
	nullptr,         // mp_length (optional)
	(binaryfunc)getitem,         // mp_subscript
	(objobjargproc)setitem       // mp_ass_subscript
};

PyTypeObject PyChangedFeature::TYPE =
{
	.tp_name = "geodesk.ChangedFeature",
	.tp_basicsize = sizeof(PyChangedFeature),
	.tp_dealloc = (destructor)dealloc,
	.tp_repr = (reprfunc)repr,
	.tp_as_mapping = &MAPPING_METHODS,
	.tp_str = (reprfunc)str,
	.tp_getattro = (getattrofunc)getattro,
	.tp_setattro = (setattrofunc)setattro,
	.tp_flags = Py_TPFLAGS_DEFAULT, // | Py_TPFLAGS_DISALLOW_INSTANTIATION,
	.tp_doc = "ChangedFeature objects",
	// .tp_richcompare = (richcmpfunc)richcompare,
};
