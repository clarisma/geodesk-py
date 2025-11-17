// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyChangedFeature.h"

#include <clarisma/util/Buffer.h>
#include <clarisma/util/DynamicStackBuffer.h>
#include <geodesk/feature/FeatureUtils.h>
#include <geodesk/feature/NodePtr.h>

#include "Changeset.h"
#include "python/Environment.h"
#include "python/geom/PyMercator.h"

#include "python/feature/PyFeature.h"
#include "python/geom/PyCoordinate.h"
#include "PyChangedMembers.h"
#include "PyChanges.h"

#include "PyCF_attr.cxx"
#include "PyCF_lookup.cxx"

PyChangedFeature* PyChangedFeature::create(Changeset* changes, Type type)
{
	PyChangedFeature* self = (PyChangedFeature*)TYPE.tp_alloc(&TYPE, 0);
	if (self) self->init(changes, type, 0);
	return self;
}

PyChangedFeature* PyChangedFeature::createNode(Changeset* changes, FixedLonLat lonLat)
{
	PyChangedFeature* self = create(changes, NODE);
	if (self)
	{
		self->original_ = nullptr;
		self->tags_ = nullptr;
		self->lon_ = lonLat.lon();
		self->lat_ = lonLat.lat();
	}
	return self;
}

PyChangedFeature* PyChangedFeature::createNode(Changeset* changes, PyAnonymousNode* node)
{
	PyChangedFeature* self = (PyChangedFeature*)TYPE.tp_alloc(&TYPE, 0);
	if (self)
	{
		self->init(changes, NODE, node->id_);
		self->original_ = Python::newRef(node);
		self->tags_ = nullptr;
		self->lon_ = Mercator::lon100ndFromX(node->x_);
		self->lat_ = Mercator::lat100ndFromY(node->y_);
	}
	return self;
}

PyChangedFeature* PyChangedFeature::create(Changeset* changes, PyFeature* feature)
{
	PyChangedFeature* self = (PyChangedFeature*)TYPE.tp_alloc(&TYPE, 0);
	if (self)
	{
		self->init(changes, static_cast<Type>(feature->feature.typeCode()), feature->feature.id());
		self->original_ = Python::newRef(feature);
		self->tags_ = nullptr;	// tags are lazy
		switch (self->type_)
		{
		case NODE:
		{
			NodePtr node(feature->feature);
			self->lon_ = Mercator::lon100ndFromX(node.x());
			self->lat_ = Mercator::lat100ndFromY(node.y());
			break;
		}
		case WAY:
			self->nodes_ = nullptr;	// lazy
			break;
		case RELATION:
			self->members_ = nullptr;	// lazy
			break;
		default:
			assert(false);
		}
	}
	return self;
}

PyChangedFeature* PyChangedFeature::createWay(Changeset* changes, PyChangedMembers* nodes)
{
	PyChangedFeature* self = create(changes, WAY);
	if (self)	[[likely]]
	{
		self->nodes_ = nodes;
	}
	else
	{
		Py_DECREF(nodes);
	}
	return self;
}

PyChangedFeature* PyChangedFeature::createRelation(Changeset* changes, PyChangedMembers* members)
{
	PyChangedFeature* self = create(changes, RELATION);
	if (self)	[[likely]]
	{
		self->members_ = members;
	}
	else
	{
		Py_DECREF(members);
	}
	return self;
}

PyChangedFeature* PyChangedFeature::createMember(PyChangedFeature* member, PyObject* role)
{
	assert(member->type_ != MEMBER);
	PyChangedFeature* self = create(member->changes_, MEMBER);
	if (self)
	{
		self->member_ = Python::newRef(member);
		self->role_ = Python::newRef(role);
	}
	return self;
}


PyChangedFeature* PyChangedFeature::create(Changeset* changes, PyObject* args, PyObject* kwargs)
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

void PyChangedFeature::init(Changeset* changes, Type type, int64_t id)
{
	changes->addref();
	changes_ = changes;
	id_ = id;
	version_ = 0;
	type_ = type;
	flags_ = Flags();
}


void PyChangedFeature::dealloc(PyChangedFeature* self)
{
	if (self->type_ == MEMBER)	[[unlikely]]
	{
		Py_XDECREF(self->member_);
		Py_XDECREF(self->role_);
	}
	else
	{
		Py_XDECREF(self->original_);
		Py_XDECREF(self->tags_);
		if (self->type_ == WAY)
		{
			Py_XDECREF(self->nodes_);
		}
		else if (self->type_ == RELATION)
		{
			Py_XDECREF(self->members_);
		}
	}
}

PyObject* PyChangedFeature::getattr(PyChangedFeature* self, PyObject *nameObj)
{
	Py_ssize_t len;
	const char* name = PyUnicode_AsUTF8AndSize(nameObj, &len);
	if (!name) return NULL;

	if (self->type_ == MEMBER)
	{
		if (std::string_view(name, len) == "role")
		{
			if (self->role_) Python::newRef(self->role_);
			Py_RETURN_NONE;
		}
		self = self->member_;
	}
	Attribute* attr = PyCF_AttrHash::lookup(name, len);
	if (!attr)
	{
		return getitem(self, nameObj);
	}
	return self->getAttribute(attr->index);
}

PyObject* PyChangedFeature::getAttribute(int attr)
{
	switch (attr)
	{
	case LAT:
		if (type_ == NODE) return PyFloat_FromDouble(lat());
		Py_RETURN_NONE;
	case LON:
		if (type_ == NODE) return PyFloat_FromDouble(lon());
		Py_RETURN_NONE;
	case MEMBERS:
		if (type_ == RELATION)
		{
			// TODO: ensure members are loaded
			return Python::newRef(members_);
		}
		Py_RETURN_NONE;
	case NODES:
		if (type_ == WAY)
		{
			if (!nodes_)
			{
				if (original_)
				{
					nodes_ = PyChangedMembers::create(changes_,
						(PyFeature*)original_);
				}
				else
				{
					nodes_ = PyChangedMembers::create(changes_, false);
				}
				if (!nodes_) return nullptr;
			}
			return Python::newRef(nodes_);
		}
		Py_RETURN_NONE;
	case ROLE:
		Py_RETURN_NONE;
	case TAGS:
		if (loadTags(true) < 0) return nullptr;
		return Python::newRef(tags_);
	case X:
		if (type_ == NODE)
		{
			return PyLong_FromLong(Mercator::xFromLon100nd(lon_));
		}
		Py_RETURN_NONE;
	case Y:
		if (type_ == NODE)
		{
			return PyLong_FromLong(Mercator::yFromLat100nd(lat_));
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
		return PyLong_FromLong(id_);
	case IS_DELETED:
		return PyBool_FromLong(has(flags_, Flags::DELETED));
	case IS_NODE:
		return PyBool_FromLong(type_ == NODE);
	case IS_RELATION:
		return PyBool_FromLong(type_ == RELATION);
	case IS_WAY:
		return PyBool_FromLong(type_ == WAY);
	case MODIFY_METHOD:
		// TODO: handle MODIFY
		Py_RETURN_NONE;
	case ORIGINAL:
		if (original_) return Python::newRef(original_);
		Py_RETURN_NONE;
	case OSM_TYPE:
		// TODO
		Py_RETURN_NONE;
	case SPLIT_METHOD:
		// TODO: handle SPLIT
		Py_RETURN_NONE;
	default:
		assert(false);
		Py_RETURN_NONE;
	}
}

bool PyChangedFeature::checkAttrType(int attr, Type type)
{
	if (type_ != type)	[[unlikely]]
	{
		PyErr_Format(PyExc_TypeError,
			"Attribute '%s' cannot be set for %s (only applies to %s)",
			ATTR_NAMES[attr], typeName(static_cast<FeatureType>(type)),
			typeName(static_cast<FeatureType>(type)));
		return false;
	}
	return true;
}


bool PyChangedFeature::setAttribute(int attr, PyObject* value)
{
	switch (attr)
	{
	case LAT:
		if (!checkAttrType(attr, NODE)) return false;
		return PyCoordinate::setLat100nd(&lat_, value);
	case LON:
		if (!checkAttrType(attr, NODE)) return false;
		return PyCoordinate::setLon100nd(&lon_, value);
	case MEMBERS:
		if (!checkAttrType(attr, RELATION)) return false;
		return setMembers(value);
	case NODES:
		if (!checkAttrType(attr, WAY)) return false;
		return setNodes(value);
	case ROLE:
		PyErr_SetString(PyExc_RuntimeError, "Add feature to a relation before assigning a role");
		return false;
	case TAGS:
		return setTags(value);
	case X:
	{
		if (!checkAttrType(attr, NODE)) return false;
		double v = PyFloat_AsDouble(value);
		if (v == -1.0 && PyErr_Occurred()) return false;
		lon_ = Mercator::lon100ndFromX(v);
		return true;
	}
	case Y:
	{
		if (!checkAttrType(attr, NODE)) return false;
		double v = PyFloat_AsDouble(value);
		if (v == -1.0 && PyErr_Occurred()) return -1;
		lat_ = Mercator::lat100ndFromY(v);
		return true;
	}
	default:
		PyErr_Format(PyExc_AttributeError,
			"Attribute '%s' is read-only", ATTR_NAMES[attr]);
		return false;
	}
}

int PyChangedFeature::setattr(PyChangedFeature* self, PyObject* nameObj, PyObject* value)
{
	Py_ssize_t len;
	const char* name = PyUnicode_AsUTF8AndSize(nameObj, &len);
	if (!name) return NULL;

	if (self->type_ == MEMBER)
	{
		if (std::string_view(name, len) == "role")
		{
			if (!Python::checkType(value, &PyUnicode_Type)) return -1;
			Py_INCREF(value);
			Py_XDECREF(self->role_);
			self->role_ = value;
			return 0;
		}
		self = self->member_;
	}
	Attribute* attr = PyCF_AttrHash::lookup(name, len);
	if (!attr)
	{
		return setitem(self, nameObj, value);
	}
	return self->setAttribute(attr->index, value) ? 0 : -1;
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
	if (self->type_ == MEMBER)	[[unlikely]]
	{
		self = self->member_;	// delegate to member
	}
	int res = self->loadTags(false);
	if (res <= 0)
	{
		if (res < 0) return nullptr;
		Py_RETURN_NONE;
	}

	// TODO: need to materialize tags!
	PyObject* value = PyDict_GetItem(self->tags_, key);
	if (!value)
	{
		Py_RETURN_NONE;
	}
	return Python::newRef(value);
}

int PyChangedFeature::setitem(PyChangedFeature* self, PyObject* key, PyObject* value)
{
	if (self->type_ == MEMBER)	[[unlikely]]
	{
		self = self->member_;	// delegate to member
	}

	// TODO: need to materialize tags!
	int res = self->loadTags(true);
	if (res < 0) return -1;
	assert(res > 0);

	// value == nullptr => deletion
	// TODO: setting to empty string should also delete
	if (value == nullptr || value == Py_None)
	{
		return PyObject_DelItem(self->tags_, key);
	}
	return PyObject_SetItem(self->tags_, key, value);
}


void PyChangedFeature::format(clarisma::Buffer& buf)
{
	if (type_ == MEMBER)	[[unlikely]]
	{
		member_->format(buf);
		buf << " as ";
		buf << Python::getStringView(role_);
		return;
	}
	buf << typeName(static_cast<FeatureType>(type_)) << '/' << id_;
	// TODO: new, anonymous
}



bool PyChangedFeature::modify(PyObject* args, PyObject* kwargs)
{
	// TODO
	return false;
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
	.tp_getattro = (getattrofunc)getattr,
	.tp_setattro = (setattrofunc)setattr,
	.tp_flags = Py_TPFLAGS_DEFAULT, // | Py_TPFLAGS_DISALLOW_INSTANTIATION,
	.tp_doc = "ChangedFeature objects",
	// .tp_richcompare = (richcmpfunc)richcompare,
};
