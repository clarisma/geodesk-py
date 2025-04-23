// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyChangedMembers.h"
#include <geodesk/feature/WayNodeIterator.h>
#include <python/feature/PyFeature.h>
#include <python/query/PyFeatures.h>

#include "PyChangedFeature.h"
#include "PyChanges.h"

// steals ref to list
PyChangedMembers* PyChangedMembers::create(PyChanges* changes, PyObject* list, bool forRelation)
{
	PyChangedMembers* self = (PyChangedMembers*)TYPE.tp_alloc(&TYPE, 0);
	if (self)	[[likely]]
	{
		self->changes = clarisma::TaggedPtr<ChangesWeakRef,1>(
			changes->newRef(), forRelation);
		self->list = list;
	}
	else
	{
		Py_DECREF(list);
	}
	return self;
}

PyChangedMembers* PyChangedMembers::create(PyChanges* changes, bool forRelation)
{
	PyObject* list = PyList_New(0);
	if (!list) return nullptr;
	return create(changes, list, forRelation);
}

PyChangedMembers* PyChangedMembers::create(PyChanges* changes, PyFeature* parent)
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


void PyChangedMembers::dealloc(PyChangedMembers* self)
{
	self->changes.ptr()->release();
	Py_DECREF(self->list);
}

PyObject* PyChangedMembers::getattro(PyChangedMembers* self, PyObject *attr)
{
	return PyObject_GetAttr(self->list, attr);
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
