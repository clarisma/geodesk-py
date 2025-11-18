// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyChanges.h"
#include <clarisma/data/HashMap.h>
#include <clarisma/io/FilePath.h>
#include "Changeset.h"
#include "ChangeSpec.h"
#include "ChangeWriter.h"
#include "PyChangedFeature.h"
#include "PyChangedMembers.h"
#include "python/feature/PyFeature.h"
#include "python/util/util.h"

using namespace clarisma;

PyChanges* PyChanges::createNew(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
	PyChanges* self = (PyChanges*)TYPE.tp_alloc(&TYPE, 0);
	if (self)
	{
		self->changes_ = new Changeset(nullptr);	// TODO: tags
	}
	return self;
}

void PyChanges::dealloc(PyChanges* self)
{
	self->changes_->clear();
	self->changes_->release();
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


PyObject* PyChanges::createFeature(PyChanges* self, PyObject* args, PyObject* kwargs)
{
	ChangeSpec spec(self->changes_,
		ChangeSpec::GEOMETRY |
		ChangeSpec::COORDINATE |
		ChangeSpec::NODES |
		ChangeSpec::MEMBERS);
	if (!spec.parse(args, 0, kwargs)) return nullptr;
	return spec.create();
}

PyObject* PyChanges::modifyFeature(PyChanges* self, PyObject* args, PyObject* kwargs)
{
	assert(PyTuple_Check(args));
	Py_ssize_t argCount = PyTuple_Size(args);
	if (argCount == 0)
	{
		PyErr_SetString(PyExc_TypeError,
			"Expected Feature or AnonymousNode");
		return nullptr;
	}
	PyChangedFeature* changed =
		(PyChangedFeature*)getitem(self, PyTuple_GET_ITEM(args, 0));
	if (!changed) return nullptr;

	ChangeSpec spec(self->changes_,
		ChangeSpec::COORDINATE |
		ChangeSpec::NODES |
		ChangeSpec::MEMBERS);
	if (!spec.parse(args, 1, kwargs)) return nullptr;
	spec.modify(changed);
	return changed;
}

PyObject* PyChanges::deleteFeature(PyChanges* self, PyObject* args, PyObject* kwargs)
{
	// Dummy implementation
	Py_RETURN_NONE;
}

PyObject* PyChanges::validate(PyChanges* self, PyObject* args, PyObject* kwargs)
{
	// Dummy implementation
	Py_RETURN_TRUE;
}

PyObject* PyChanges::save(PyChanges* self, PyObject* args, PyObject* kwargs)
{
	PyObject* fileNameObj = Python::checkSingleArg(args, kwargs, &PyUnicode_Type);
	if (!fileNameObj) return nullptr;
	std::string_view fileName = Python::stringAsStringView(fileNameObj);
	std::string fileNameWithExt = FilePath::withDefaultExtension(fileName, ".osc");
	ChangeWriter writer(fileNameWithExt.c_str());
	writer.write(self->changes_);
	Py_RETURN_NONE;
}

PyObject* PyChanges::getitem(PyChanges* self, PyObject* key)
{
	if (Py_TYPE(key) == &PyFeature::TYPE)
	{
		return self->changes_->modify((PyFeature*)key);
	}
	if (Py_TYPE(key) == &PyAnonymousNode::TYPE)
	{
		return self->changes_->modify((PyAnonymousNode*)key);
	}
	PyErr_SetString(PyExc_TypeError, "Expected Feature or AnonymousNode");
	return nullptr;
}


PyMappingMethods PyChanges::MAPPING_METHODS =
{
	nullptr,         // mp_length (optional)
	(binaryfunc)getitem,         // mp_subscript
	nullptr       // mp_ass_subscript
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
	// .tp_getattro = (getattrofunc)getattro,
	.tp_flags = Py_TPFLAGS_DEFAULT, // | Py_TPFLAGS_DISALLOW_INSTANTIATION,
	.tp_doc = "Changes objects",
	.tp_methods = METHODS,
	.tp_new = (newfunc)createNew,
	// .tp_richcompare = (richcmpfunc)richcompare,
};

