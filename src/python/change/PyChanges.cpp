// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyChanges.h"
#include "PyChangedFeature.h"

#include <clarisma/data/HashMap.h>
#include "python/feature/PyFeature.h"
#include "python/util/util.h"

PyChanges* PyChanges::createNew(PyTypeObject* type, PyObject* args, PyObject* kwargs)
{
	PyChanges* self = (PyChanges*)TYPE.tp_alloc(&TYPE, 0);
	if (self)
	{
		// TODO: may throw (make RAII?)
		new(&self->newAnonNodes)FeaturesByCoordinate();
		new(&self->existingAnonNodes)FeaturesByCoordinate();
		new(&self->features)FeaturesByTypedId();
		self->tags = nullptr;
		self->weakRef = new ChangesWeakRef(self);
			// TODO: handle OOM
	}
	return self;
}

void PyChanges::dealloc(PyChanges* self)
{
	self->newAnonNodes.~FeaturesByCoordinate();
	self->existingAnonNodes.~FeaturesByCoordinate();
	self->features.~FeaturesByTypedId();
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

PyChangedFeature* PyChanges::createNode(Coordinate xy)
{
	auto it = newAnonNodes.find(xy);
	if (it != newAnonNodes.end())
	{
		return Python::newRef(it->second);
	}
	PyChangedFeature* changed = PyChangedFeature::create(this, xy);
	if (!changed) return nullptr;
	newAnonNodes[xy] = changed;
	return Python::newRef(changed);
}

PyChangedFeature* PyChanges::modify(FeatureStore* store, uint64_t id, Coordinate xy)
{
	auto it = existingAnonNodes.find(xy);
	if (it != existingAnonNodes.end())
	{
		return Python::newRef(it->second);
	}
	PyChangedFeature* changed = PyChangedFeature::create(this,
		PyAnonymousNode::create(store, xy.x, xy.y));		// TODO: node ID
	if (!changed) return nullptr;
	existingAnonNodes[xy] = changed;
	return Python::newRef(changed);
}

PyChangedFeature* PyChanges::modify(PyAnonymousNode* node)
{
	Coordinate xy(node->x_, node->y_);
	auto it = existingAnonNodes.find(xy);
	if (it != existingAnonNodes.end())
	{
		return Python::newRef(it->second);
	}
	PyChangedFeature* changed = PyChangedFeature::create(this, node);
	if (!changed) return nullptr;
	existingAnonNodes[xy] = changed;
	return Python::newRef(changed);
}

PyChangedFeature* PyChanges::modify(PyFeature* feature)
{
	TypedFeatureId typedId = feature->feature.typedId();
	auto it = features.find(typedId);
	if (it != features.end())
	{
		return Python::newRef(it->second);
	}
	PyChangedFeature* changed = PyChangedFeature::create(this, feature);
	if (!changed) return nullptr;
	features[typedId] = changed;
	return Python::newRef(changed);
}

PyObject* PyChanges::createFeature(PyChanges* self, PyObject* args, PyObject* kwargs)
{
	// Dummy implementation
	Py_RETURN_NONE;
}

PyObject* PyChanges::modifyFeature(PyChanges* self, PyObject* args, PyObject* kwargs)
{
	// Dummy implementation
	Py_RETURN_NONE;
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
	// Dummy implementation
	Py_RETURN_NONE;
}

PyObject* PyChanges::getitem(PyChanges* self, PyObject* key)
{
	if (Py_TYPE(key) == &PyFeature::TYPE)
	{
		return self->modify((PyFeature*)key);
	}
	if (Py_TYPE(key) == &PyAnonymousNode::TYPE)
	{
		return self->modify((PyAnonymousNode*)key);
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
	.tp_getattro = (getattrofunc)getattro,
	.tp_flags = Py_TPFLAGS_DEFAULT, // | Py_TPFLAGS_DISALLOW_INSTANTIATION,
	.tp_doc = "Changes objects",
	.tp_methods = METHODS,
	.tp_new = (newfunc)createNew,
	// .tp_richcompare = (richcmpfunc)richcompare,
};
