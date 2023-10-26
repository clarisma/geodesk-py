// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <Python.h>
#include <structmember.h>

class PyBinder
{
public:
	static const int MAX_TARGETS = 16;

	PyObject_HEAD
	PyObject* targets[MAX_TARGETS];
	int targetCount;

	static PyTypeObject TYPE;
	static PyMappingMethods MAPPING_METHODS;

	static PyBinder* create();
	static void dealloc(PyBinder* self);
	static PyObject* subscript(PyBinder* self, PyObject* key);

	void addTarget(PyObject* t);
	void popTarget();
};
