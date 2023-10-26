// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyFastMethod.h"
#include <exception>
#include <new>

PyObject* PyFastMethod::create(PyObject* obj, PyCFunctionWithKeywords func)
{
	PyFastMethod* self = (PyFastMethod*)(TYPE.tp_alloc(&TYPE, 0));
	if (self)
	{
		Py_INCREF(obj);
		self->object = obj;
		self->function = func;
	}
	return (PyObject*)self;
}

PyObject* PyFastMethod::call(PyFastMethod* self, PyObject* args, PyObject* kwargs)
{
	try
	{
		return self->function(self->object, args, kwargs);
	}
	catch (const std::bad_alloc&)
	{
		PyErr_SetNone(PyExc_MemoryError);
	}
	catch (const std::exception& ex)
	{
		PyErr_SetString(PyExc_RuntimeError, ex.what());
	}
	return NULL;
}

void PyFastMethod::dealloc(PyFastMethod* self)
{
	Py_DECREF(self->object);
	Py_TYPE(self)->tp_free(self);
}


PyTypeObject PyFastMethod::TYPE =
{
	.tp_name = "geodesk.FastMethod",
	.tp_basicsize = sizeof(PyFastMethod),
	.tp_dealloc = (destructor)dealloc,
	.tp_call = (ternaryfunc)call,
	.tp_flags = Py_TPFLAGS_DEFAULT, // | Py_TPFLAGS_DISALLOW_INSTANTIATION,
	.tp_doc = "FastMethod objects",
};
