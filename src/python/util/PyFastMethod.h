// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <Python.h>
#include <structmember.h>

class PyFastMethod
{
public:
	PyObject_HEAD
	PyObject* object;
	PyCFunctionWithKeywords function;

	static PyTypeObject TYPE;
	
	static PyObject* create(PyObject* obj, PyCFunctionWithKeywords func);
	static PyObject* call(PyFastMethod* self, PyObject* args, PyObject* kwargs);
	static void dealloc(PyFastMethod* self);
};
