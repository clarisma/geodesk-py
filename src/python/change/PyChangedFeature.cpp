// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyChangedFeature.h"

#include <clarisma/util/Buffer.h>
#include <clarisma/util/DynamicStackBuffer.h>
#include <geodesk/feature/FeatureUtils.h>
#include <geodesk/feature/NodePtr.h>
#include <geodesk/feature/WayNodeIterator.h>
#include "Changeset.h"
#include "python/Environment.h"
#include "python/geom/PyCoordinate.h"
#include "python/feature/PyFeature.h"
#include "PyCF_attr.cxx"
#include "PyCF_lookup.cxx"

PyChangedFeature* PyChangedFeature::create(Changeset* changes, int type)
{
	PyChangedFeature* self = (PyChangedFeature*)TYPE.tp_alloc(&TYPE, 0);
	if (self) self->init(changes, (Type)type, 0);
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
		switch (self->type())
		{
		case NODE:
		{
			NodePtr node(feature->feature);
			self->lon_ = Mercator::lon100ndFromX(node.x());
			self->lat_ = Mercator::lat100ndFromY(node.y());
			break;
		}
		case WAY:
		case RELATION:
			self->children_ = nullptr;	// lazy
			break;
		default:
			assert(false);
		}
	}
	return self;
}

PyChangedFeature* PyChangedFeature::createFeature2D(
	Changeset* changes, int type, PyObject* children)
{
	PyChangedFeature* self = create(changes, type);
	if (self)	[[likely]]
	{
		self->children_ = children;
	}
	else
	{
		Py_DECREF(children);
	}
	return self;
}


PyObject* PyChangedFeature::createChildren(
	Changeset* changes, PyObject* seq, bool forRelation)
{
	Py_ssize_t n = PySequence_Fast_GET_SIZE(seq);
	PyObject **items = PySequence_Fast_ITEMS(seq);
	PyObject* list = PyList_New(n);

	for (int i=0; i<n; i++)
	{
		PyChangedFeature* changed = promoteChild(
			changes, items[i], forRelation);
		if (!changed)
		{
			Py_DECREF(list);
			return nullptr;
		}
		if (!forRelation)
		{
			if (changed->type() != NODE)
			{
				forRelation = true;
				// Upgrade any preceding nodes to member
				for (int i2=0; i2<i; i2++)
				{
					PyChangedFeature* prev = (PyChangedFeature*)
						PyList_GET_ITEM(list, i2);
					assert(prev->type() == PyChangedFeature::NODE);
					PyList_SET_ITEM(list, i2,
						PyChangedFeature::createMember(prev, Py_None));
				}
			}
		}
		if (forRelation && changed->type() != MEMBER)
		{
			changed = createMember(changed, Py_None);
		}
		PyList_SET_ITEM(list, i, changed);
	}
	return list;
}



PyChangedFeature* PyChangedFeature::createMember(PyChangedFeature* member, PyObject* role)
{
	// TODO: upgrade "None" to empty string ???

	assert(!member->isMember());
	PyChangedFeature* self = create(member->changes_, MEMBER);
	if (self)
	{
		self->member_ = Python::newRef(member);
		self->role_ = Python::newRef(role);
	}
	return self;
}

/*
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
*/

void PyChangedFeature::init(Changeset* changes, Type type, int64_t id)
{
	changes->addref();
	changes_ = changes;
	idAndFlags_ = (id << FLAG_COUNT) | type;
	version_ = 0;
	usageCount_ = 0;
}


void PyChangedFeature::dealloc(PyChangedFeature* self)
{
	if (self->isMember())	[[unlikely]]
	{
		Py_XDECREF(self->member_);
		Py_XDECREF(self->role_);
	}
	else
	{
		Py_XDECREF(self->original_);
		Py_XDECREF(self->tags_);
		if (self->type() != NODE)
		{
			Py_XDECREF(self->children_);
		}
	}
	self->changes_->release();
	Py_TYPE(self)->tp_free(self);
}

PyObject* PyChangedFeature::getattr(PyChangedFeature* self, PyObject *nameObj)
{
	Py_ssize_t len;
	const char* name = PyUnicode_AsUTF8AndSize(nameObj, &len);
	if (!name) return NULL;

	if (self->isMember())
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

PyObject* PyChangedFeature::loadNodes(Changeset* changes, FeatureStore* store, WayPtr way)
{
	WayNodeIterator iter(store, way, true, store->hasWaynodeIds());
	int count = iter.remaining();
	PyObject* list = PyList_New(count);
	if (!list) return nullptr;
	for (int i=0;i<count;i++)
	{
		PyChangedFeature* node = nullptr;
		WayNodeIterator::WayNode wayNode = iter.next();
		if (wayNode.feature.isNull())	[[likely]]
		{
			node = changes->modify(store, wayNode.id, wayNode.xy);
		}
		else
		{
			PyFeature* featureNode = PyFeature::create(store, wayNode.feature, Py_None);
			if (featureNode)
			{
				node = changes->modify(featureNode);
				Py_DECREF(featureNode);
			}
		}
		if (!node)	[[unlikely]]
		{
			Py_DECREF(list);
			return nullptr;
		}
		PyList_SET_ITEM(list, i, node);  // steals reference to node
	}
	return list;
}

PyObject* PyChangedFeature::loadMembers(Changeset* changes, FeatureStore* store, RelationPtr rel)
{
	PyObject* list = PyList_New(0);
	if (!list) return nullptr;

	MemberIterator iter(store, rel.bodyptr());
	for (;;)
	{
		FeaturePtr feature = iter.next();
		if (feature.isNull()) break;
		PyObject* role = iter.borrowCurrentRole();
		PyFeature* featureObj = PyFeature::create(store, feature, role);
		if (featureObj)
		{
			PyChangedFeature* changed = changes->modify(featureObj);
			Py_DECREF(featureObj);
			if (changed)
			{
				PyChangedFeature* member = createMember(changed, role);
				if (member)
				{
					if (PyList_Append(list, member) == 0) continue;
				}
			}
		}
		Py_DECREF(list);
		return nullptr;
	}
	return list;
}

PyListProxy* PyChangedFeature::getNodes()
{
	assert(type() == WAY);
	if (!children_)
	{
		assert(original_);
		PyFeature* feature = (PyFeature*)original_;
		children_ = loadNodes(changes_, feature->store,
			WayPtr(feature->feature));
		if (!children_) return nullptr;
	}
	return PyListProxy::create(children_, this, &CHILD_OPERATIONS);
}

PyListProxy* PyChangedFeature::getMembers()
{
	assert(type() == RELATION);
	if (!children_)
	{
		assert(original_);
		PyFeature* feature = (PyFeature*)original_;
		children_ = loadMembers(changes_, feature->store,
			RelationPtr(feature->feature));
		if (!children_) return nullptr;
	}
	return PyListProxy::create(children_, this, &CHILD_OPERATIONS);
}


PyObject* PyChangedFeature::getAttribute(int attr)
{
	switch (attr)
	{
	case LAT:
		if (type() == NODE) return PyFloat_FromDouble(lat());
		Py_RETURN_NONE;
	case LON:
		if (type() == NODE) return PyFloat_FromDouble(lon());
		Py_RETURN_NONE;
	case MEMBERS:
		if (type() == RELATION) return getMembers();
		Py_RETURN_NONE;
	case NODES:
		if (type() == WAY) return getNodes();
		Py_RETURN_NONE;
	case ROLE:
		Py_RETURN_NONE;
	case TAGS:
		if (loadTags(true) < 0) return nullptr;
		return Python::newRef(tags_);
	case X:
		if (type() == NODE)
		{
			return PyLong_FromLong(Mercator::xFromLon100nd(lon_));
		}
		Py_RETURN_NONE;
	case Y:
		if (type() == NODE)
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
		return PyLong_FromLong(id());
	case IS_DELETED:
		return PyBool_FromLong(idAndFlags_ & Flags::DELETED);
	case IS_NODE:
		return PyBool_FromLong(type() == NODE);
	case IS_RELATION:
		return PyBool_FromLong(type() == RELATION);
	case IS_WAY:
		return PyBool_FromLong(type() == WAY);
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

bool PyChangedFeature::checkAttrType(int attr, Type requiredType) const
{
	if (type() != requiredType)	[[unlikely]]
	{
		PyErr_Format(PyExc_TypeError,
			"Attribute '%s' cannot be set for %s (only applies to %s)",
			ATTR_NAMES[attr], typeName(featureType()),
			typeName(static_cast<FeatureType>(requiredType)));
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

	if (self->isMember())
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
	if (self->isMember())	[[unlikely]]
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
	if (self->isMember())	[[unlikely]]
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


void PyChangedFeature::format(clarisma::Buffer& buf) const
{
	if (isMember())	[[unlikely]]
	{
		member_->format(buf);
		if(role_ != Py_None)
		{
			auto strRole = Python::getStringView(role_);
			if(!strRole.empty()) buf << " as " << strRole;
		}
		return;
	}
	buf << typeName(featureType()) << '/' << id();
	// TODO: new, anonymous
}



bool PyChangedFeature::modify(PyObject* args, PyObject* kwargs)
{
	// TODO
	return false;
}


PyObject* PyChangedFeature::append(PyObject* self_, PyObject* arg)
{
    auto self = ASSERT_PYTHON_TYPE(self_, PyChangedFeature);
    return PyListProxy::append(self->children_, arg,
        self, &coerceChild, &childAdded);
}


PyObject* PyChangedFeature::extend(PyObject* self_, PyObject* arg)
{
	auto self = ASSERT_PYTHON_TYPE(self_, PyChangedFeature);
	return PyListProxy::extend(self->children_, arg,
		self, &coerceChild, &childAdded);
}

PyObject* PyChangedFeature::insert(PyObject* self_, PyObject* args)
{
    auto self = ASSERT_PYTHON_TYPE(self_, PyChangedFeature);
    return PyListProxy::insert(self->children_, args,
		self, &coerceChild, &childAdded);
}


PyObject* PyChangedFeature::remove(PyObject* self_, PyObject* arg)
{
    auto self = ASSERT_PYTHON_TYPE(self_, PyChangedFeature);
    return PyListProxy::remove(self->children_, arg,
		self, &childEquals, &childRemoved);
}


PyObject* PyChangedFeature::remove_all(PyObject* self_, PyObject* arg)
{
    auto self = ASSERT_PYTHON_TYPE(self_, PyChangedFeature);
	return PyListProxy::removeAll(self->children_, arg,
		self, &childEquals, &childRemoved);
}


PyObject* PyChangedFeature::pop(PyObject* self_, PyObject* args)
{
    auto self = ASSERT_PYTHON_TYPE(self_, PyChangedFeature);
    return PyListProxy::pop(self->children_, args,
        self, &childRemoved);
}


PyObject* PyChangedFeature::clear(PyObject* self_, PyObject* /*ignored*/)
{
    auto self = ASSERT_PYTHON_TYPE(self_, PyChangedFeature);
    return PyListProxy::clear(self->children_, self, &childRemoved);
}


PyObject* PyChangedFeature::reverse(PyObject* self_, PyObject* /*ignored*/)
{
    auto self = ASSERT_PYTHON_TYPE(self_, PyChangedFeature);
    return PyListProxy::reverse(self->children_, self, &childrenReordered);
}


PyObject* PyChangedFeature::count(PyObject* self_, PyObject* arg)
{
    auto self = ASSERT_PYTHON_TYPE(self_, PyChangedFeature);
    return PyListProxy::count(self->children_, arg, &childEquals);
}


int PyChangedFeature::contains(PyObject* self_, PyObject* arg)
{
    auto self = ASSERT_PYTHON_TYPE(self_, PyChangedFeature);
    return PyListProxy::contains(self->children_, arg, &childEquals);
}


PyObject* PyChangedFeature::index(PyObject* self_, PyObject* args)
{
    auto self = ASSERT_PYTHON_TYPE(self_, PyChangedFeature);
    return PyListProxy::index(self->children_, args, &childEquals);
}

PyChangedFeature* PyChangedFeature::promoteChild(Changeset* changes, PyObject *obj, bool withRole)
{
	PyChangedFeature* child = changes->tryModify(obj);
	if(child) return child;
	if(PyErr_Occurred()) return nullptr;
	PyObject *seq = PySequence_Fast(obj,
		 withRole ?
			"Expected feature, coordinate or member tuple" :
			"Expected feature or coordinate");
	if (!seq) return nullptr;
	Py_ssize_t n = PySequence_Fast_GET_SIZE(seq);
	if (n != 2)
	{
		Py_DECREF(seq);
		PyErr_SetString(PyExc_TypeError,
			withRole ?
				"Expected (feature,role) or coordinate" :
				"Expected coordinate");
		return nullptr;
	}
	PyObject* first = PySequence_Fast_GET_ITEM(seq, 0);
	PyObject* second = PySequence_Fast_GET_ITEM(seq, 1);
	Py_DECREF(seq); // TODO: safe ???
	if (withRole && PyUnicode_Check(second))
	{
		// TODO: also allow (role,feature) ?
		child = promoteChild(changes, first, false);
		if(!child) return nullptr;
		return createMember(child, second);
	}
	return changes->createNode(first, second);
}


PyChangedFeature* PyChangedFeature::coerceChild(Changeset* changes, PyObject* item, bool forRelation)
{
	PyChangedFeature* changed = promoteChild(changes, item, forRelation);
	if (!changed) return nullptr;
	if (forRelation)
	{
		if (!changed->isMember())
		{
			changed = createMember(changed, Py_None);
		}
	}
	else
	{
		if (changed->type() != NODE)
		{
			PyErr_SetString(PyExc_TypeError, "Expected node");
			Py_DECREF(changed);
			return nullptr;
		}
	}
	return changed;
}

PyObject* PyChangedFeature::coerceChild(PyObject* parent, PyObject* item)
{
	auto self = ASSERT_PYTHON_TYPE(parent, PyChangedFeature);
	assert(self->type() == WAY || self->type() == RELATION);
	return coerceChild(self->changes_, item, self->type() == RELATION);
}


void PyChangedFeature::childAdded(PyObject* parent,
	PyObject* list, PyObject* item)
{
	auto self = ASSERT_PYTHON_TYPE(parent, PyChangedFeature);
	(void)self;
	(void)list;
	(void)item;

	// TODO: handle child added
}


void PyChangedFeature::childRemoved(PyObject* parent,
	PyObject* list,	PyObject* item)
{
	auto self = ASSERT_PYTHON_TYPE(parent, PyChangedFeature);
	(void)self;
	(void)list;
	(void)item;

	// TODO: handle child removed
}


bool PyChangedFeature::childEquals(PyObject* item, PyObject* other)
{
	// Default: use Python == semantics
	int rc = PyObject_RichCompareBool(item, other, Py_EQ);
	if (rc < 0)
	{
		PyErr_Clear();
		return false;
	}
	return rc != 0;
}


void PyChangedFeature::childrenReordered(PyObject* parent, PyObject* list)
{
	auto self = ASSERT_PYTHON_TYPE(parent, PyChangedFeature);
	(void)self;
	(void)list;

	// TODO: handle children reordered
}


const PyListProxy::Operations PyChangedFeature::CHILD_OPERATIONS =
{
	coerceChild,
	childEquals,
	childAdded,
	childRemoved,
	childrenReordered
};

PyMethodDef PyChangedFeature::METHODS[] =
{
	{
		"append", append,	METH_O,
		"Append node/member"
	},
	{
		"extend", extend,	METH_O,
		"Extend with nodes/members from an iterable"
	},
	{
		"insert", insert,	METH_VARARGS,
		"Insert node/member at index"
	},
	{
		"remove", remove,	METH_O,
		"Remove first occurrence of ndoe/member"
	},
	{
		"pop", pop,METH_VARARGS,
		"Remove and return node/member at index (default last)"
	},
	{
		"clear", clear, METH_NOARGS,
		"Remove all nodes/members"
	},
	{
		"reverse",	reverse, METH_NOARGS,
		"Reverse the nodes/members in place"
	},
	{
		"count", count, METH_O,
		"Count occurrences of a node/member"
	},
	{
		"index", index,METH_VARARGS,
		"Return first index of a node/member"
	},
	{ nullptr, nullptr, 0, nullptr }
};



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
