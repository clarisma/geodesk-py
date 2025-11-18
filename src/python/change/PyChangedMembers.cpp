// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyChangedMembers.h"
#include <geodesk/feature/WayNodeIterator.h>
#include <python/feature/PyFeature.h>
#include <python/query/PyFeatures.h>
#include "PyChangedFeature.h"
#include "Changeset.h"
#include "python/geom/PyCoordinate.h"
#include "python/geom/PyMercator.h"

// steals ref to list
PyChangedMembers* PyChangedMembers::create(Changeset* changes, PyObject* list, bool forRelation)
{
	PyChangedMembers* self = (PyChangedMembers*)TYPE.tp_alloc(&TYPE, 0);
	if (self)	[[likely]]
	{
		changes->addref();
		self->changes = clarisma::TaggedPtr<Changeset,1>(changes, forRelation);
		self->list = list;
	}
	else
	{
		Py_DECREF(list);
	}
	return self;
}

PyChangedMembers* PyChangedMembers::create(Changeset* changes, bool forRelation)
{
	PyObject* list = PyList_New(0);
	if (!list) return nullptr;
	return create(changes, list, forRelation);
}

PyChangedMembers* PyChangedMembers::create(Changeset* changes, PyFeature* parent)
{
	PyObject* list;
	FeatureStore* store = parent->store;
	bool forRelation;
	if (parent->feature.isWay())	[[likely]]
	{
		forRelation = false;
		WayNodeIterator iter(store, WayPtr(parent->feature),
			true, store->hasWaynodeIds());
		int count = iter.remaining();
		list = PyList_New(count);
		if (!list) return nullptr;
		for (int i=0;i<count;i++)
		{
			PyChangedFeature* node;
			WayNodeIterator::WayNode wayNode = iter.next();
			if (wayNode.feature.isNull())	[[likely]]
			{
				node = changes->modify(store, wayNode.id, wayNode.xy);
			}
			else
			{
				PyFeature* featureNode = PyFeature::create(store, wayNode.feature, Py_None);
				node = changes->modify(featureNode);
				Py_DECREF(featureNode);
			}
			if (!node)	[[unlikely]]
			{
				Py_DECREF(list);
				return nullptr;
			}
			PyList_SET_ITEM(list, i, node);  // steals reference to node
		}
	}
	else
	{
		assert(parent->feature.isRelation());
		forRelation = true;
		// TODO
	}
	return create(changes, list, forRelation);
}


bool PyChangedMembers::tryAcceptMember(PyChangedFeature** changed, Changeset* changes, PyObject* obj)
{
	if (Py_TYPE(obj) == &PyChangedFeature::TYPE)
	{
		PyChangedFeature* feature = (PyChangedFeature*)obj;
		if (feature->type() == PyChangedFeature::MEMBER)
		{
			feature = feature->member();
		}
		*changed = feature;
		return true;
	}
	if (Py_TYPE(obj) == &PyFeature::TYPE)
	{
		*changed = changes->modify((PyFeature*)obj);
		return true;
	}
	if (Py_TYPE(obj) == &PyAnonymousNode::TYPE)
	{
		*changed = changes->modify((PyAnonymousNode*)obj);
		return true;
	}
	if (Py_TYPE(obj) == &PyCoordinate::TYPE)
	{
		*changed = changes->createNode(((PyCoordinate*)obj)->coordinate());
		return true;
	}
	return false;
}

PyChangedFeature* PyChangedMembers::acceptMember(Changeset* changes, PyObject* obj)
{
	PyChangedFeature* changed;
	if (tryAcceptMember(&changed, changes, obj)) return changed;

	PyObject *seq = PySequence_Fast(obj,
		"Expected feature, coordinate or member tuple");
	if (!seq) return nullptr;
	Py_ssize_t n = PySequence_Fast_GET_SIZE(seq);
	if (n != 2)
	{
		Py_DECREF(seq);
		PyErr_SetString(PyExc_TypeError, "Expected (feature,role) or coordinate");
		return nullptr;
	}
	PyObject* first = PySequence_Fast_GET_ITEM(seq, 0);
	PyObject* second = PySequence_Fast_GET_ITEM(seq, 1);
	Py_DECREF(seq); // TODO: safe ???
	if (PyUnicode_Check(second))
	{
		// TODO: also allow (role,feature) ?
		if (!tryAcceptMember(&changed, changes, first)) return nullptr;
		return PyChangedFeature::createMember(changed, second);
	}
	return changes->createNode(first, second);
}


PyChangedMembers* PyChangedMembers::fromSequence(Changeset* changes, PyObject* seq, bool forRelation)
{
	Py_ssize_t n = PySequence_Fast_GET_SIZE(seq);
	PyObject **items = PySequence_Fast_ITEMS(seq);
	PyObject* list = PyList_New(n);

	for (int i=0; i<n; i++)
	{
		PyChangedFeature* changed = acceptMember(changes, items[i]);
		if (!changed)
		{
			Py_DECREF(list);
			return nullptr;
		}
		if (!forRelation)
		{
			if (changed->type() != PyChangedFeature::NODE)
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
		if (forRelation && changed->type() != PyChangedFeature::MEMBER)
		{
			changed = PyChangedFeature::createMember(changed, Py_None);
		}
		PyList_SET_ITEM(list, i, changed);
	}
	return create(changes, list, forRelation);
}

void PyChangedMembers::dealloc(PyChangedMembers* self)
{
	self->changes.ptr()->release();
	Py_DECREF(self->list);
}

PyObject* PyChangedMembers::getattro(PyChangedMembers* self, PyObject *attr)
{
	return PyObject_GetAttr(self->list, attr);
}

PyObject* PyChangedMembers::getitem(PyChangedMembers* self, PyObject* key)
{
	return PyObject_GetItem(self->list, key);
}

int PyChangedMembers::setitem(PyChangedMembers* self, PyObject* key, PyObject* value)
{
	// TODO
	/*
	if (value == nullptr)
	{
		// Handle deletion: del obj[key]
		return PyObject_DelItem(self->items_, key);
	}
	else
	{
		// Handle assignment: obj[key] = value
		return PyObject_SetItem(self->items_, key, value);
	}
	*/
	return 0;
}

PyObject* PyChangedMembers::iter(PyChangedMembers* self)
{
	return PyObject_GetIter(self->list);
}

PyObject* PyChangedMembers::repr(PyChangedMembers* self)
{
	return PyObject_Repr(self->list);
}

PyObject* PyChangedMembers::richcompare(PyChangedMembers* self, PyObject* other, int op)
{
	return PyObject_RichCompare(self->list, other, op);
}

PyObject* PyChangedMembers::str(PyChangedMembers* self)
{
	return PyObject_Str(self->list);
}

/*
PyMethodDef PyChangedMembers::METHODS[] =
{
	{"save", (PyCFunction)save, METH_VARARGS, "Saves the file" },
	{ NULL, NULL, 0, NULL },
};
*/


PySequenceMethods PyChangedMembers::SEQUENCE_METHODS =
{
};

PyMappingMethods PyChangedMembers::MAPPING_METHODS =
{
	nullptr,					 // mp_length (optional)
	(binaryfunc)getitem,         // mp_subscript
	(objobjargproc)setitem       // mp_ass_subscript
};

PyTypeObject PyChangedMembers::TYPE =
{
	.tp_name = "geodesk.ChangedMembers",
	.tp_basicsize = sizeof(PyChangedMembers),
	.tp_dealloc = (destructor)dealloc,
	.tp_repr = (reprfunc)repr,
	.tp_as_sequence = &SEQUENCE_METHODS,
	.tp_as_mapping = &MAPPING_METHODS,
	.tp_str = (reprfunc)str,
	.tp_getattro = (getattrofunc)getattro,
	.tp_flags = Py_TPFLAGS_DEFAULT, // | Py_TPFLAGS_DISALLOW_INSTANTIATION,
	.tp_doc = "ChangedMembers objects",
	.tp_richcompare = (richcmpfunc)richcompare,
	.tp_iter = (getiterfunc)iter,
	/*
	.tp_methods = METHODS,
	.tp_members = MEMBERS,
	.tp_getset = GETSET,
	*/
};
