// Copyright (c) 2025 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyChangedChildren.h"
#include <geodesk/feature/WayNodeIterator.h>
#include <python/feature/PyFeature.h>
#include <python/query/PyFeatures.h>
#include "PyChangedFeature.h"
#include "Changeset.h"
#include "python/geom/PyCoordinate.h"
#include "python/geom/PyMercator.h"


PyChangedFeature* PyChangedChildren::promoteChild(Changeset* changes, PyObject *obj, bool withRole)
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
		return PyChangedFeature::createMember(child, second);
	}
	return changes->createNode(first, second);
}


// steals ref to list
PyChangedChildren* PyChangedChildren::create(Changeset* changes, PyObject* list, bool forRelation)
{
	PyChangedChildren* self = (PyChangedChildren*)TYPE.tp_alloc(&TYPE, 0);
	if (self)	[[likely]]
	{
		changes->addref();
		self->changes_ = clarisma::TaggedPtr<Changeset,1>(changes, forRelation);
		self->list_ = list;
	}
	else
	{
		Py_DECREF(list);
	}
	return self;
}

PyChangedChildren* PyChangedChildren::create(Changeset* changes, bool forRelation)
{
	PyObject* list = PyList_New(0);
	if (!list) return nullptr;
	return create(changes, list, forRelation);
}

PyChangedChildren* PyChangedChildren::create(Changeset* changes, PyFeature* parent)
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

PyChangedChildren* PyChangedChildren::fromSequence(Changeset* changes, PyObject* seq, bool forRelation)
{
	Py_ssize_t n = PySequence_Fast_GET_SIZE(seq);
	PyObject **items = PySequence_Fast_ITEMS(seq);
	PyObject* list = PyList_New(n);

	for (int i=0; i<n; i++)
	{
		PyChangedFeature* changed = promoteChild(changes, items[i], true);
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

void PyChangedChildren::dealloc(PyChangedChildren* self)
{
	self->changes()->release();
	Py_DECREF(self->list_);
}


PyObject* PyChangedChildren::getitem(PyChangedChildren* self, PyObject* key)
{
	return PyObject_GetItem(self->list_, key);
}

int PyChangedChildren::setitem(PyChangedChildren* self, PyObject* key, PyObject* value)
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

PyObject* PyChangedChildren::iter(PyChangedChildren* self)
{
	return PyObject_GetIter(self->list_);
}

PyObject* PyChangedChildren::repr(PyChangedChildren* self)
{
	return PyObject_Repr(self->list_);
}

PyObject* PyChangedChildren::richcompare(PyChangedChildren* self, PyObject* other, int op)
{
	return PyObject_RichCompare(self->list_, other, op);
}

PyObject* PyChangedChildren::str(PyChangedChildren* self)
{
	return PyObject_Str(self->list_);
}

/*
PyMethodDef PyChangedMembers::METHODS[] =
{
	{"save", (PyCFunction)save, METH_VARARGS, "Saves the file" },
	{ NULL, NULL, 0, NULL },
};
*/


PySequenceMethods PyChangedChildren::SEQUENCE_METHODS =
{
};

PyMappingMethods PyChangedChildren::MAPPING_METHODS =
{
	nullptr,					 // mp_length (optional)
	(binaryfunc)getitem,         // mp_subscript
	(objobjargproc)setitem       // mp_ass_subscript
};

PyTypeObject PyChangedChildren::TYPE =
{
	.tp_name = "geodesk.ChangedChildren",
	.tp_basicsize = sizeof(PyChangedChildren),
	.tp_dealloc = (destructor)dealloc,
	.tp_repr = (reprfunc)repr,
	.tp_as_sequence = &SEQUENCE_METHODS,
	.tp_as_mapping = &MAPPING_METHODS,
	.tp_str = (reprfunc)str,
	.tp_flags = Py_TPFLAGS_DEFAULT, // | Py_TPFLAGS_DISALLOW_INSTANTIATION,
	.tp_doc = "ChangedChildren objects",
	.tp_richcompare = (richcmpfunc)richcompare,
	.tp_iter = (getiterfunc)iter,
	/*
	.tp_methods = METHODS,
	.tp_members = MEMBERS,
	.tp_getset = GETSET,
	*/
};
