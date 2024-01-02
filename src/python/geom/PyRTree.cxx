// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyRTree.h"
#include "PyBox.h"
#include "geom/rtree/HilbertTreeBuilder.h"
#include "python/util/util.h"

PyObject* PyRTree::create(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
	Py_ssize_t argCount = PySequence_Length(args);
	if (argCount != 1 || kwargs != NULL)
	{
		PyErr_SetString(PyExc_TypeError, "Expected single argument (iterable)");
		return NULL;
	}
	PyObject* arg = PyTuple_GET_ITEM(args, 0);
	PyObject* seq = PySequence_Fast(arg, "Expected sequence or iterable");
	if (!seq) return NULL;
	Py_ssize_t itemCount = PySequence_Fast_GET_SIZE(seq);

	BoundedItems items(itemCount);

	PyObject** seqItems = PySequence_Fast_ITEMS(seq);
	for (int i = 0; i < itemCount; i++)
	{
		PyObject* item = seqItems[i];
		if (PyTuple_Check(item))
		{
			Py_ssize_t tupleSize = PyTuple_Size(item);
			if (tupleSize == 2)
			{
				PyObject* box = PyTuple_GET_ITEM(item, 0);
				if (box->ob_type == &PyBox::TYPE)
				{
					items.add((PyBox*)box, PyTuple_GET_ITEM(item, 1));
					continue;
				}
			}
		}
		else
		{
			PyObject* box = PyObject_GetAttrString(item, "bounds");
			if (box)
			{
				if (box->ob_type == &PyBox::TYPE)
				{
					items.add((PyBox*)box, item);
					Py_DECREF(box);
					continue;
				}
				Py_DECREF(box);
			}
		}

		items.releaseItems();
		PyErr_Format(PyExc_TypeError, "Item #%d: Expected object with 'bounds' or (Box, item) tuple instead of %s",
			i, item->ob_type->tp_name);
		Py_DECREF(seq);
		return NULL;
	}

	PyRTree* self = (PyRTree*)TYPE.tp_alloc(&TYPE, 0);
	if (self)
	{
		HilbertTreeBuilder builder(nullptr);
		self->tree = builder.build<PyObject>(items.data(), items.size(), 9, Box()); // TODO: configurable
	}
	Py_DECREF(seq);
	return self;
}

void PyRTree::dealloc(PyRTree* self)
{
	self->tree.~RTree();		// Call destructor explicitly
	Py_TYPE(self)->tp_free(self);
}


PyObject* PyRTree::subscript(PyRTree* self, PyObject* key)
{
	PyBox* box = NULL;	// TODO
	return PyRTreeQuery::create(self, box->box);
}


PyObject* PyRTree::root(PyRTree* self, void*)
{
	// TODO
	Py_RETURN_NONE;
}


PyObject* PyRTree::str(PyRTree* self)
{
	// TODO
	Py_RETURN_NONE;
}

/*
PyMethodDef PyRTree::METHODS[] =
{
	{"save", (PyCFunction)save, METH_VARARGS, "Saves the file" },
	{ NULL, NULL, 0, NULL },
};
*/


// This causes problems because PyGetSetDef fields aren't const in earlier
// versions of Python
// must use const_cast<char*>() to cast them
PyGetSetDef PyRTree::GETSET[] =
{
	{ "root", (getter)root, (setter)NULL, "Root node of this r-tree", NULL },
	{ NULL, NULL, NULL, NULL, NULL },
};


PyMappingMethods PyRTree::MAPPING_METHODS =
{
	.mp_subscript = (binaryfunc)PyRTree::subscript
};

PyTypeObject PyRTree::TYPE =
{
	.tp_name = "geodesk.RTree",
	.tp_basicsize = sizeof(PyRTree),
	.tp_dealloc = (destructor)dealloc,
	.tp_as_mapping = &MAPPING_METHODS,
	.tp_str = (reprfunc)str,
	.tp_flags = Py_TPFLAGS_DEFAULT,
	.tp_doc = "RTree objects",
	// .tp_methods = METHODS,
	.tp_getset = GETSET,
	.tp_new = PyRTree::create
};


PyRTreeQuery* PyRTreeQuery::create(PyRTree* tree, const Box& bounds)
{
	// const RTree<PyObject*>::Node* root;
	PyRTreeQuery* self = (PyRTreeQuery*)TYPE.tp_alloc(&TYPE, 0);
	if (self)
	{
		Py_INCREF(tree);
		self->target = tree;
		self->bounds = bounds;
		self->currentLevel = 0;
		self->inLeaf = false;
	}
	return self;
}

void PyRTreeQuery::dealloc(PyRTreeQuery* self)
{
	Py_DECREF(self->target);
	Py_TYPE(self)->tp_free(self);
}

PyObject* PyRTreeQuery::next(PyRTreeQuery* self)
{
	// TODO
	Py_RETURN_NONE;
}

PyTypeObject PyRTreeQuery::TYPE =
{
	.tp_name = "geodesk.RTreeQuery",
	.tp_basicsize = sizeof(PyRTreeQuery),
	.tp_dealloc = (destructor)dealloc,
	.tp_flags = Py_TPFLAGS_DEFAULT, // | Py_TPFLAGS_DISALLOW_INSTANTIATION,
	.tp_iter = PyObject_SelfIter,
	.tp_iternext = (iternextfunc)next,
};


PyRTree::BoundedItems::BoundedItems(size_t expectedCount)
{
	reserve(expectedCount);
}

void PyRTree::BoundedItems::add(PyBox* box, PyObject* item)
{
	BoundedItem boundedItem;
	boundedItem.bounds = box->box;
	boundedItem.item = Python::newRef(item);
	push_back(boundedItem);
}

void PyRTree::BoundedItems::releaseItems()
{
	for (auto it = begin(); it != end(); ++it)
	{
		Py_DECREF((PyObject*)(*it).item);
	}
}

