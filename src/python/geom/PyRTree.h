// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <Python.h>
#include <structmember.h>
#include <vector>
#include "geom/Box.h"
#include "geom/rtree/RTree.h"

class PyBox;

class PyRTree : public PyObject
{
public:
	RTree<PyObject> tree;

	static PyTypeObject TYPE;
	// static PyMethodDef METHODS[];
	static PyGetSetDef GETSET[];
	static PyMappingMethods MAPPING_METHODS;

	static PyObject* create(PyTypeObject* type, PyObject* args, PyObject* kwargs);
	static void dealloc(PyRTree* self);
	static PyObject* subscript(PyRTree* self, PyObject* key);
	static PyObject* str(PyRTree* self);

	static PyObject* root(PyRTree* self , void*);

	class BoundedItems : public std::vector<BoundedItem>
	{
	public:
		BoundedItems(size_t expectedCount);
		
		void add(PyBox* box, PyObject* item);
		void releaseItems();
	};
};


class PyRTreeQuery : public PyObject
{
public:
	static const int MAX_STACK_DEPTH = 16;

	PyRTree* target;
	Box bounds;
	const RTree<PyObject>::Node* stack[MAX_STACK_DEPTH];
	int currentLevel;
	bool inLeaf;

	static PyTypeObject TYPE;

	static PyRTreeQuery* create(PyRTree* tree, const Box& bounds);
	static void dealloc(PyRTreeQuery* self);
	static PyObject* next(PyRTreeQuery* self);
};
