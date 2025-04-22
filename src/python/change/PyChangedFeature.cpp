// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyChangedFeature.h"

PyObject* PyChangedFeature::call(PyChangedFeature* self, PyObject* args, PyObject* kwargs)
{
	// TODO
	Py_RETURN_NONE;
}

void PyChangedFeature::dealloc(PyChangedFeature* self)
{
	// TODO
}

PyObject* PyChangedFeature::getattro(PyChangedFeature* self, PyObject *attr)
{
	// TODO
	Py_RETURN_NONE;
}

Py_hash_t PyChangedFeature::hash(PyChangedFeature* self)
{
	// TODO
	return 0;
}

PyObject* PyChangedFeature::iter(PyChangedFeature* self)
{
	// TODO
	Py_RETURN_NONE;
}

PyObject* PyChangedFeature::next(PyChangedFeature* self)
{
	// TODO
	Py_RETURN_NONE;
}

PyObject* PyChangedFeature::repr(PyChangedFeature* self)
{
	// TODO
	Py_RETURN_NONE;
}

PyObject* PyChangedFeature::richcompare(PyChangedFeature* self, PyObject* other, int op)
{
	// TODO
	Py_RETURN_NONE;
}

PyObject* PyChangedFeature::str(PyChangedFeature* self)
{
	// TODO
	Py_RETURN_NONE;
}

PyMethodDef PyChangedFeature::METHODS[] =
{
	{"save", (PyCFunction)save, METH_VARARGS, "Saves the file" },
	{ NULL, NULL, 0, NULL },
};

PyMemberDef PyChangedFeature::MEMBERS[] =
{
	{"first", T_OBJECT_EX, offsetof(PyChangedFeature, first), 0, "first name"},
	{ NULL, 0, 0, 0, NULL },
};

PyGetSetDef PyChangedFeature::GETSET[] =
{
	{ "value", (getter)get_value, (setter)set_value, "Description of value", NULL },
	{ NULL, NULL, NULL, NULL, NULL },
};

PyNumberMethods PyChangedFeature::NUMBER_METHODS =
{
};

PySequenceMethods PyChangedFeature::SEQUENCE_METHODS =
{
};

PyMappingMethods PyChangedFeature::MAPPING_METHODS =
{
};

PyTypeObject PyChangedFeature::TYPE =
{
	.tp_name = "geodesk.ChangedFeature",
	.tp_basicsize = sizeof(PyChangedFeature),
	.tp_dealloc = (destructor)dealloc,
	.tp_repr = (reprfunc)repr,
	.tp_as_number = &NUMBER_METHODS,
	.tp_as_sequence = &SEQUENCE_METHODS,
	.tp_as_mapping = &MAPPING_METHODS,
	.tp_hash = (hashfunc)hash,
	.tp_call = (ternaryfunc)call,
	.tp_str = (reprfunc)str,
	.tp_getattro = (getattrofunc)getattro,
	.tp_flags = Py_TPFLAGS_DEFAULT, // | Py_TPFLAGS_DISALLOW_INSTANTIATION,
	.tp_doc = "ChangedFeature objects",
	.tp_richcompare = (richcmpfunc)richcompare,
	.tp_iter = (getiterfunc)iter,
	.tp_iternext = (iternextfunc)next,
	.tp_methods = METHODS,
	.tp_members = MEMBERS,
	.tp_getset = GETSET,
};
