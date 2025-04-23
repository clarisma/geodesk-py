// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyChangedFeature.h"

#include <clarisma/util/Buffer.h>
#include <clarisma/util/DynamicStackBuffer.h>
#include <geodesk/feature/FeatureUtils.h>
#include <geodesk/feature/NodePtr.h>
#include <geodesk/feature/TagIterator.h>
#include <python/geom/PyMercator.h>

#include "python/feature/PyFeature.h"
#include "python/geom/PyCoordinate.h"
#include "PyChangedMembers.h"
#include "PyChanges.h"

#include "PyChangedFeature_attr.cxx"
#include "PyChangedFeature_lookup.cxx"

PyChangedFeature* PyChangedFeature::create(PyChanges* changes, Coordinate xy)
{
	PyChangedFeature* self = (PyChangedFeature*)TYPE.tp_alloc(&TYPE, 0);
	if (self)
	{
		self->init(changes, NODE, 0);
		self->original = nullptr;
		self->tags = nullptr;
		self->x = xy.x;
		self->y = xy.y;
	}
	return self;
}

PyChangedFeature* PyChangedFeature::create(PyChanges* changes, PyAnonymousNode* node)
{
	PyChangedFeature* self = (PyChangedFeature*)TYPE.tp_alloc(&TYPE, 0);
	if (self)
	{
		self->init(changes, NODE, 0);	// TODO: in v2, anon nodes can have id
		self->original = Python::newRef(node);
		self->tags = nullptr;
		self->x = node->x_;
		self->y = node->y_;
	}
	return self;
}

PyChangedFeature* PyChangedFeature::create(PyChanges* changes, PyFeature* feature)
{
	PyChangedFeature* self = (PyChangedFeature*)TYPE.tp_alloc(&TYPE, 0);
	if (self)
	{
		self->init(changes, static_cast<Type>(feature->feature.typeCode()), feature->feature.id());
		self->original = Python::newRef(feature);
		self->tags = nullptr;	// tags are lazy
		switch (self->type)
		{
		case NODE:
		{
			NodePtr node(feature->feature);
			self->x = node.x();
			self->y = node.y();
			break;
		}
		case WAY:
			self->nodes = nullptr;	// lazy
			break;
		case RELATION:
			self->members = nullptr;	// lazy
			break;
		default:
			assert(false);
		}
	}
	return self;
}

void PyChangedFeature::init(PyChanges* changes_, Type type_, int64_t id_)
{
	changes = changes_->newRef();
	id = id_;
	version = 0;
	type = type_;
	isDeleted = false;
	maybeHasNewParents = false;
}

void PyChangedFeature::dealloc(PyChangedFeature* self)
{
	if (self->type == MEMBER)
	{
		Py_XDECREF(self->member);
		Py_XDECREF(self->role);
	}
	else
	{
		Py_XDECREF(self->original);
		Py_XDECREF(self->tags);
		if (self->type == WAY)
		{
			Py_XDECREF(self->nodes);
		}
		else if (self->type == RELATION)
		{
			Py_XDECREF(self->members);
		}
	}
}

PyObject* PyChangedFeature::getattro(PyChangedFeature* self, PyObject *nameObj)
{
	Py_ssize_t len;
    const char* name = PyUnicode_AsUTF8AndSize(nameObj, &len);
	if (!name) return NULL;

	if (self->type == MEMBER)
	{
		if (std::string_view(name, len) == "role")
		{
			if (self->role) Python::newRef(self->role);
			Py_RETURN_NONE;
		}
		self = self->member;
	}
	Attribute* attr = PyChangedFeature_AttrHash::lookup(name, len);
	if (!attr)
	{
		return getitem(self, nameObj);
	}
	int type = self->type;
	switch (attr->index)
	{
	case LAT:
		if (type == NODE) return PyCoordinate::niceLatFromY(self->y);
		Py_RETURN_NONE;
	case LON:
		if (type == NODE) return PyCoordinate::niceLonFromX(self->x);
		Py_RETURN_NONE;
	case MEMBERS:
		if (type == RELATION)
		{
			// TODO: ensure members are loaded
			return Python::newRef(self->members);
		}
		Py_RETURN_NONE;
	case NODES:
		if (type == WAY)
		{
			if (!self->nodes)
			{
				PyChanges* changes = self->changes->getOrRaise();
				if (!changes) return nullptr;
				if (self->original)
				{
					self->nodes = PyChangedMembers::create(changes,
						(PyFeature*)self->original);
				}
				else
				{
					self->nodes = PyChangedMembers::create(changes, false);
				}
				if (!self->nodes) return nullptr;
			}
			return Python::newRef(self->nodes);
		}
		Py_RETURN_NONE;
	case ROLE:
		Py_RETURN_NONE;
	case SHAPE:
		// TODO
		Py_RETURN_NONE;
	case TAGS:
		if (self->loadTags(true) < 0) return nullptr;
		return Python::newRef(self->tags);
	case X:
		if (type == NODE) return PyLong_FromLong(self->x);
		Py_RETURN_NONE;
	case Y:
		if (type == NODE) return PyLong_FromLong(self->y);
		Py_RETURN_NONE;
	case COMBINE_METHOD:
		// TODO: handle COMBINE
		Py_RETURN_NONE;
	case CONNECT_METHOD:
		// TODO: handle CONNECT
		Py_RETURN_NONE;
	case DELETE_METHOD:
		// TODO: handle DELETE
		Py_RETURN_NONE;
	case ID:
		return PyLong_FromLong(self->id);
	case IS_DELETED:
		return PyBool_FromLong(self->isDeleted);
	case IS_NODE:
		return PyBool_FromLong(type == NODE);
	case IS_RELATION:
		return PyBool_FromLong(type == RELATION);
	case IS_WAY:
		return PyBool_FromLong(type == WAY);
	case MODIFY_METHOD:
		// TODO: handle MODIFY
		Py_RETURN_NONE;
	case ORIGINAL:
		if (self->original) return Python::newRef(self->original);
		Py_RETURN_NONE;
	case OSM_TYPE:
		// TODO
		Py_RETURN_NONE;
	case SPLIT_METHOD:
		// TODO: handle SPLIT
		Py_RETURN_NONE;
	default:
		assert(false);
		break;
	}
}

void PyChangedFeature::wrongAttrForType(int attr, Type only)
{
	PyErr_Format(PyExc_TypeError,
		"Attribute '%s' cannot be set for %s (only applies to %s)",
		ATTR_NAMES[attr], typeName(static_cast<FeatureType>(type)),
		typeName(static_cast<FeatureType>(only)));
}

bool PyChangedFeature::applyAttr(int attr, Type only)
{
	if (type != only)
	{
		if (type != UNASSIGNED)
		{
			wrongAttrForType(attr, only);
			return false;
		}
		type = only;
	}
	return true;
}

int PyChangedFeature::setProperty(int attr, PyObject* value)
{
	switch (attr)
	{
	case LAT:
		if (!applyAttr(attr, NODE)) return -1;
		return PyMercator::setYFromLat(&y, value) ? GROUP_Y : -1;
	case LON:
		if (!applyAttr(attr, NODE)) return -1;
		return PyMercator::setXFromLon(&x, value) ? GROUP_X : -1;
	case MEMBERS:
		if (!applyAttr(attr, RELATION)) return -1;
		return setMembers(value) ? GROUP_SHAPE : -1;
	case NODES:
		if (!applyAttr(attr, WAY)) return -1;
		return setNodes(value) ? GROUP_SHAPE : -1;
	case ROLE:
		PyErr_SetString(PyExc_RuntimeError, "Add feature to a relation before assigning a role");
		return -1;
	case SHAPE:
		return setShape(value) ? GROUP_SHAPE : -1;
	case TAGS:
		return setTags(value) ? GROUP_TAGS_ALL : -1;
	case X:
	{
		if (!applyAttr(attr, NODE)) return -1;
		double v = PyFloat_AsDouble(value);
		if (v == -1.0 && PyErr_Occurred()) return -1;
		x = (int32_t)round(v);
		return GROUP_X;
	}
	case Y:
	{
		if (!applyAttr(attr, NODE)) return -1;
		double v = PyFloat_AsDouble(value);
		if (v == -1.0 && PyErr_Occurred()) return -1;
		y = (int32_t)round(v);
		return GROUP_Y;
	}
	default:
		PyErr_Format(PyExc_AttributeError,
			"Attribute '%s' is read-only", ATTR_NAMES[attr]);
		return -1;
	}
}

int PyChangedFeature::setattro(PyChangedFeature* self, PyObject* nameObj, PyObject* value)
{
	Py_ssize_t len;
	const char* name = PyUnicode_AsUTF8AndSize(nameObj, &len);
	if (!name) return NULL;

	if (self->type == MEMBER)
	{
		if (std::string_view(name, len) == "role")
		{
			if (!Python::checkType(value, &PyUnicode_Type)) return -1;
			Py_INCREF(value);
			Py_XDECREF(self->role);
			self->role = value;
			return 0;
		}
		self = self->member;
	}
	Attribute* attr = PyChangedFeature_AttrHash::lookup(name, len);
	if (!attr)
	{
		return setitem(self, nameObj, value);
	}
	return self->setProperty(attr->index, value);
}

bool PyChangedFeature::setMembers(PyObject* value)
{
	// TODO
	return false;
}

bool PyChangedFeature::setNodes(PyObject* value)
{
	// TODO
	return false;
}

bool PyChangedFeature::setShape(PyObject* value)
{
	// TODO
	return false;
}

bool PyChangedFeature::setTags(PyObject* value)
{
	// TODO
	return false;
}

PyObject* PyChangedFeature::repr(PyChangedFeature* self)
{
	clarisma::DynamicStackBuffer<1024> buf;
	self->format(buf);
	return PyUnicode_FromStringAndSize(buf.data(), buf.length());
}

/*
PyObject* PyChangedFeature::richcompare(PyChangedFeature* self, PyObject* other, int op)
{
	// TODO
	Py_RETURN_NONE;
}
*/

PyObject* PyChangedFeature::str(PyChangedFeature* self)
{
	return repr(self);
}

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
