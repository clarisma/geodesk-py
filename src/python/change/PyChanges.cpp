// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyChanges.h"

#include <clarisma/data/HashMap.h>

PyChanges* PyChanges::createNew(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
	PyChanges* self = (PyChanges*)TYPE.tp_alloc(&TYPE, 0);
	if (self)
	{
		new(self->anonNodes)clarisma::HashMap();
		new(self->features)clarisma::HashMap();
		self->tags = nullptr;
		self->weakRef = new ChangesWeakRef(self);
			// TODO: handle OOM
	}
	return self;
}

void PyChanges::dealloc(PyChanges* self)
{
	self->anonNodes.~clarisma::HashMap();
	self->features.~clarisma::HashMap();
	Py_XDECREF(self->tags);
	self->weakRef->clear();
	self->weakRef->release();
}

PyObject* PyChanges::getattro(PyChanges* self, PyObject *attr)
{
	// TODO
	Py_RETURN_NONE;
}


/*
PyObject* PyChanges::repr(PyChanges* self)
{
	// TODO
	Py_RETURN_NONE;
}

PyObject* PyChanges::richcompare(PyChanges* self, PyObject* other, int op)
{
	// TODO
	Py_RETURN_NONE;
}
 */

PyObject* PyChanges::str(PyChanges* self)
{
	// TODO
	Py_RETURN_NONE;
}

PyMappingMethods PyChanges::MAPPING_METHODS =
{
};

PyMethodDef PyChanges::METHODS[] =
{
	{
		"create",
		(PyCFunction)createFeature,
		METH_VARARGS | METH_KEYWORDS,
		"Create a new feature"
	},
	{
		"modify",
		(PyCFunction)modifyFeature,
		METH_VARARGS | METH_KEYWORDS,
		"Modify a feature"
	},
	{
		"delete",
		(PyCFunction)deleteFeature,
		METH_VARARGS | METH_KEYWORDS,
		"Delete a feature"
	},
	{
		"validate",
		(PyCFunction)validate,
		METH_VARARGS | METH_KEYWORDS,
		"Validate all changes"
	},
	{
		"save",
		(PyCFunction)save,
		METH_VARARGS | METH_KEYWORDS,
		"Save changes as .osc file"
	},
	{nullptr, nullptr, 0, nullptr}
};


PyTypeObject PyChanges::TYPE =
{
	.tp_name = "geodesk.Changes",
	.tp_basicsize = (Py_ssize_t)sizeof(PyChanges),
	.tp_dealloc = (destructor)dealloc,
	// .tp_repr = (reprfunc)repr,
	.tp_as_mapping = &MAPPING_METHODS,
	.tp_str = (reprfunc)str,
	.tp_getattro = (getattrofunc)getattro,
	.tp_flags = Py_TPFLAGS_DEFAULT, // | Py_TPFLAGS_DISALLOW_INSTANTIATION,
	.tp_doc = "Changes objects",
	.tp_methods = METHODS,
	.tp_new = (newfunc)createNew,
	// .tp_richcompare = (richcmpfunc)richcompare,
};
