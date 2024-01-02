// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyBinder.h"
#include "python/util/util.h"

PyBinder* PyBinder::create()
{
    PyBinder* self = (PyBinder*)TYPE.tp_alloc(&TYPE, 0);
    if (self != nullptr)
    {
        self->targetCount = 0;
    }
    return self;
}

void PyBinder::dealloc(PyBinder* self)
{
	for (int i = 0; i < self->targetCount; i++)
	{
		Py_DECREF(self->targets[i]);
	}
	Py_TYPE(self)->tp_free(self);
}

void PyBinder::addTarget(PyObject* t)
{
	assert(targetCount < MAX_TARGETS - 1);
	targets[targetCount++] = Python::newRef(t);
}

void PyBinder::popTarget()
{
	assert(targetCount > 0);
	Py_DECREF(targets[--targetCount]);
}

PyObject* PyBinder::subscript(PyBinder* self, PyObject* key)
{
	/*
	if (!PyUnicode_Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "Key must be a string");
		return NULL;
	}
	*/

	for(int i=self->targetCount-1; i>=0; i--)
	{
		PyObject* target = self->targets[i];
		int hasAttr = PyObject_HasAttr(target, key);
		if (hasAttr == 1)
		{
			return PyObject_GetAttr(target, key);
		}
	}
	Py_RETURN_NONE;
}

PyMappingMethods PyBinder::MAPPING_METHODS =
{
	nullptr,         // mp_length
	reinterpret_cast<binaryfunc>(PyBinder::subscript), // mp_subscript
	nullptr          // mp_ass_subscript
};

PyTypeObject PyBinder::TYPE =
{
	.tp_name = "geodesk.Binder",
	.tp_basicsize = sizeof(PyBinder),
	.tp_dealloc = (destructor)dealloc,
	.tp_as_mapping = &MAPPING_METHODS,
	.tp_flags = Py_TPFLAGS_DEFAULT, // | Py_TPFLAGS_DISALLOW_INSTANTIATION,
	.tp_doc = "Binder objects",
};
