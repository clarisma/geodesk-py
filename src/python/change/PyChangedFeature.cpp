// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyChangedFeature.h"

#include <clarisma/util/Buffer.h>
#include <clarisma/util/DynamicStackBuffer.h>
#include <geodesk/feature/FeatureUtils.h>
#include <geodesk/feature/NodePtr.h>
#include <python/geom/PyMercator.h>

#include "python/feature/PyFeature.h"
#include "python/geom/PyCoordinate.h"
#include "PyChangedMembers.h"
#include "PyChanges.h"

#include "PyChangedFeature_attr.cxx"
#include "PyChangedFeature_lookup.cxx"

PyChangedFeature* PyChangedFeature::create(PyChanges* changes, Type type)
{
	PyChangedFeature* self = (PyChangedFeature*)TYPE.tp_alloc(&TYPE, 0);
	if (self) self->init(changes, type, 0);
	return self;
}

PyChangedFeature* PyChangedFeature::create(PyChanges* changes, FixedLonLat lonLat)
{
	PyChangedFeature* self = create(changes, NODE);
	if (self)
	{
		self->original = nullptr;
		self->tags = nullptr;
		self->lon = lonLat.lon();
		self->lat = lonLat.lat();
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
		self->lon = Mercator::lon100ndFromX(node->x_);
		self->lat = Mercator::lat100ndFromY(node->y_);
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
			self->lon = Mercator::lon100ndFromX(node.x());
			self->lat = Mercator::lat100ndFromY(node.y());
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

PyChangedFeature* PyChangedFeature::createMember(PyChangedFeature* member, PyObject* role)
{
	assert(member->type != MEMBER);
	PyChanges* changes = member->changes->getOrRaise();
	if (!changes) return nullptr;
	PyChangedFeature* self = create(changes, MEMBER);
	if (self)
	{
		self->member = member;
		self->role = Python::newRef(role);
	}
	return self;
}


PyChangedFeature* PyChangedFeature::create(PyChanges* changes, PyObject* args, PyObject* kwargs)
{
	PyChangedFeature* self = create(changes, UNASSIGNED);
	if (self)
	{
		if (!self->modify(args, kwargs))
		{
			Py_DECREF(self);
			return nullptr;
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
		if (type == NODE) return PyFloat_FromDouble(self->lat / 1e7);
		Py_RETURN_NONE;
	case LON:
		if (type == NODE) return PyFloat_FromDouble(self->lon / 1e7);
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
		if (type == NODE)
		{
			return PyLong_FromLong(Mercator::xFromLon100nd(self->lon));
		}
		Py_RETURN_NONE;
	case Y:
		if (type == NODE)
		{
			return PyLong_FromLong(Mercator::yFromLat100nd(self->lat));
		}
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
		return PyCoordinate::setLat100nd(&lat, value) ? GROUP_Y : -1;
	case LON:
		if (!applyAttr(attr, NODE)) return -1;
		return PyCoordinate::setLon100nd(&lon, value) ? GROUP_X : -1;
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
		lon = Mercator::lon100ndFromX(v);
		return GROUP_X;
	}
	case Y:
	{
		if (!applyAttr(attr, NODE)) return -1;
		double v = PyFloat_AsDouble(value);
		if (v == -1.0 && PyErr_Occurred()) return -1;
		lat = Mercator::lat100ndFromY(v);
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
	return self->setProperty(attr->index, value) < 0 ? -1 : 0;
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


bool PyChangedFeature::Builder::pushNode()
{
	if (!list_)
	{
		list_ = PyList_New(0);
		if (!list_) return false;
	}
	PyChangedFeature* node = changes_->createNode(
		PyMercator::getAgnosticCoordinate(xOrLon_, yOrLat_));

	if (!node) return false;
	if (isMemberList_)
	{
		node = createMember(node,
			Environment::get().getString(Environment::Strings::BLANK));
	}
	if (PyList_Append(list_, node) < 0)
	{
		Py_DECREF(node);
		return false;
	}
	Py_DECREF(node);
	return true;
}

/// @brief Attempts to turn the given value into a ChangedFeature
/// Accepts ChangedFeature (in which case it simply returns a new ref),
/// Feature/AnonymousNode, Coordinate, Shapely geometry, or a tuple
/// from which it attempts to create a feature
///
/// @returns	1 if a feature ws successfully created
///				0 if value is not suitable for a feature
///				-1 if an error occurred (exception is set)
int PyChangedFeature::Builder::tryCreateFeature(PyObject* value, PyChangedFeature** feature)
{
	PyTypeObject* type = Py_TYPE(value);
	if (type == &PyChangedFeature::TYPE)
	{
		*feature = Python::newRef((PyChangedFeature*)value);
		return 1;
	}
	if (type == &PyFeature::TYPE)
	{
		*feature = create(changes_, (PyFeature*)value);
		return *feature ? 1 : -1;
	}
	if (type == &PyAnonymousNode::TYPE)
	{
		*feature = create(changes_, (PyAnonymousNode*)value);
		return *feature ? 1 : -1;
	}
	if (type == &PyCoordinate::TYPE)
	{
		PyCoordinate* coord = (PyCoordinate*)value;
		*feature = changes_->createNode(Coordinate(coord->x, coord->y));
		return *feature ? 1 : -1;
	}

	// TODO: Shapely geometry
	return 0;
}

bool PyChangedFeature::Builder::build(
	PyChangedFeature* feature, PyObject* args, PyObject* kwargs)
{
	feature_ = feature;
	changes_ = feature->changes->getOrRaise();
	if (!changes_) return false;

	assert(PyTuple_Check(args));
	Py_ssize_t argCount = PyTuple_Size(args);
	for (Py_ssize_t i=0; i<argCount; i++)
	{
		PyObject* arg = PyTuple_GetItem(args, i); // borrowed ref
		if (PyFloat_Check(arg) || PyLong_Check(arg))
		{
			if (coordValueCount_ == 2)
			{
				// We've staged a full coordinate
				// -> create a node and append to list
				pushNode();
				coordValueCount_ = 0;
			}
			double val = PyFloat_AsDouble(arg);
			if (coordValueCount_ == 0)
			{
				xOrLon_ = val;
			}
			else
			{
				assert(coordValueCount_ == 1);
				yOrLat_ = val;
			}
			coordValueCount_++;
			continue;
		}
		if (PyUnicode_Check(arg))		// String (key of tag)
		{
			// handle string
			continue;
		}
		PyChangedFeature* child;
		int res = tryCreateFeature(arg, &child);
		if (res < 0) return false;
		if (res == 1)
		{

		}
		if (PyTuple_Check(arg))		// coord pair, member or feature
		{
			// handle tuple
			continue;
		}
		else if (Py_TYPE(arg) == &PyCoordinate::TYPE)
		{
			// handle PyCoordinate*
			auto* coord = reinterpret_cast<PyCoordinate*>(arg);
			// use coord->x_, coord->y_, etc.
		}

		// --- List
		else if (PyList_Check(arg))
		{
			// handle list
		}

		// --- Dict
		else if (PyDict_Check(arg))
		{
			// handle dict
		}

		// --- Shapely geometry object
		else if (PyObject_HasAttrString(arg, "geom_type"))  // quick way to test
		{
			// handle Shapely geometry
		}

		else
		{
			PyErr_Format(PyExc_TypeError,
						 "Unexpected argument type: %.200s",
						 Py_TYPE(arg)->tp_name);
			return false;
		}
	}
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
