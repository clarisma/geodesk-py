// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyChangedMembers.h"

PyObject* PyChangedMembers::call(PyChangedMembers* self, PyObject* args, PyObject* kwargs)
{
	// TODO
	Py_RETURN_NONE;
}

void PyChangedMembers::dealloc(PyChangedMembers* self)
{
	// TODO
}

PyObject* PyChangedMembers::getattro(PyChangedMembers* self, PyObject *attr)
{
	// TODO
	Py_RETURN_NONE;
}

Py_hash_t PyChangedMembers::hash(PyChangedMembers* self)
{
	// TODO
	return 0;
}

PyObject* PyChangedMembers::iter(PyChangedMembers* self)
{
	// TODO
	Py_RETURN_NONE;
}

PyObject* PyChangedMembers::next(PyChangedMembers* self)
{
	// TODO
	Py_RETURN_NONE;
}

PyObject* PyChangedMembers::repr(PyChangedMembers* self)
{
	// TODO
	Py_RETURN_NONE;
}

PyObject* PyChangedMembers::richcompare(PyChangedMembers* self, PyObject* other, int op)
{
	// TODO
	Py_RETURN_NONE;
}

PyObject* PyChangedMembers::str(PyChangedMembers* self)
{
	// TODO
	Py_RETURN_NONE;
}

PyMethodDef PyChangedMembers::METHODS[] =
{
	{"save", (PyCFunction)save, METH_VARARGS, "Saves the file" },
	{ NULL, NULL, 0, NULL },
};

PyMemberDef PyChangedMembers::MEMBERS[] =
{
	{"first", T_OBJECT_EX, offsetof(PyChangedMembers, first), 0, "first name"},
	{ NULL, 0, 0, 0, NULL },
};

PyGetSetDef PyChangedMembers::GETSET[] =
{
	{ "value", (getter)get_value, (setter)set_value, "Description of value", NULL },
	{ NULL, NULL, NULL, NULL, NULL },
};

PyNumberMethods PyChangedMembers::NUMBER_METHODS =
{
};

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
	.tp_as_number = &NUMBER_METHODS,
	.tp_as_sequence = &SEQUENCE_METHODS,
	.tp_as_mapping = &MAPPING_METHODS,
	.tp_hash = (hashfunc)hash,
	.tp_call = (ternaryfunc)call,
	.tp_str = (reprfunc)str,
	.tp_getattro = (getattrofunc)getattro,
	.tp_flags = Py_TPFLAGS_DEFAULT, // | Py_TPFLAGS_DISALLOW_INSTANTIATION,
	.tp_doc = "ChangedMembers objects",
	.tp_richcompare = (richcmpfunc)richcompare,
	.tp_iter = (getiterfunc)iter,
	.tp_iternext = (iternextfunc)next,
	.tp_methods = METHODS,
	.tp_members = MEMBERS,
	.tp_getset = GETSET,
};
