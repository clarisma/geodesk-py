// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyChanges.h"
#include <clarisma/data/HashMap.h>
#include <clarisma/io/FilePath.h>
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
		new(&self->model_)ChangeModel();
		self->tags = nullptr;
		self->weakRef = new ChangesWeakRef(self);
			// TODO: handle OOM
	}
	return self;
}

void PyChanges::dealloc(PyChanges* self)
{
	self->model_.~ChangeModel();
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

PyChangedFeature* PyChanges::createNode(FixedLonLat lonLat)
{
	auto it = model_.createdAnonNodes.find(lonLat);
	if (it != model_.createdAnonNodes.end())
	{
		// TODO: check if node has been moved or tagged,
		//  in which case we create a new one
		return Python::newRef(it->second.get());
	}
	PyChangedFeature* changed = PyChangedFeature::create(this, lonLat);
	if (!changed) return nullptr;
	model_.createdAnonNodes[lonLat] = PyFeatureRef(changed);
	return Python::newRef(changed);
}

PyChangedFeature* PyChanges::createWay(PyObject* nodeList)	// steals ref
{
	PyChangedMembers* nodes = PyChangedMembers::create(this, nodeList, false);
		// steals ref to nodeList even if it fails
	if (!nodes) return nullptr;
	PyChangedFeature* way =  PyChangedFeature::create(this, PyChangedFeature::WAY);
	if (!way)
	{
		Py_DECREF(nodes);
		return nullptr;
	}
	way->nodes = nodes;
	// TODO: add to list of new features
	return way;
}

PyChangedFeature* PyChanges::createRelation(PyObject* memberList)
{
	PyChangedMembers* members = PyChangedMembers::create(this, memberList, true);
	// steals ref to nodeList even if it fails
	if (!members) return nullptr;
	PyChangedFeature* rel =  PyChangedFeature::create(this, PyChangedFeature::RELATION);
	if (!rel)
	{
		Py_DECREF(members);
		return nullptr;
	}
	rel->members = members;
	// TODO: add to list of new features
	return rel;
}

PyChangedFeature* PyChanges::modify(FeatureStore* store, uint64_t id, Coordinate xy)
{
	auto it = model_.existingAnonNodes.find(xy);
	if (it != model_.existingAnonNodes.end())
	{
		return Python::newRef(it->second.get());
	}
	PyChangedFeature* changed = PyChangedFeature::create(this,
		PyAnonymousNode::create(store, id, xy.x, xy.y));
	if (!changed) return nullptr;
	model_.existingAnonNodes[xy] = PyFeatureRef(changed);
	return Python::newRef(changed);
}

PyChangedFeature* PyChanges::modify(PyAnonymousNode* node)
{
	Coordinate xy(node->x_, node->y_);
	auto it = model_.existingAnonNodes.find(xy);
	if (it != model_.existingAnonNodes.end())
	{
		return Python::newRef(it->second.get());
	}
	PyChangedFeature* changed = PyChangedFeature::create(this, node);
	if (!changed) return nullptr;
	model_.existingAnonNodes[xy] = PyFeatureRef(changed);
	return Python::newRef(changed);
}

PyChangedFeature* PyChanges::modify(PyFeature* feature)
{
	uint64_t id = feature->feature.id();
	auto& features = model_.existing[feature->feature.typeCode()];
	auto it = features.find(id);
	if (it != features.end())
	{
		return Python::newRef(it->second.get());
	}
	PyChangedFeature* changed = PyChangedFeature::create(this, feature);
	if (!changed) return nullptr;
	features[id] = PyFeatureRef(changed);
	return Python::newRef(changed);
}

PyObject* PyChanges::createFeature(PyChanges* self, PyObject* args, PyObject* kwargs)
{
	PyChangedFeature::Parameters params(self,
		PyChangedFeature::Parameters::GEOMETRY |
		PyChangedFeature::Parameters::COORDINATE |
		PyChangedFeature::Parameters::NODES |
		PyChangedFeature::Parameters::MEMBERS);
	if (!params.parse(args, 0, kwargs)) return nullptr;

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
	PyObject* fileNameObj = Python::checkSingleArg(args, kwargs, &PyUnicode_Type);
	if (!fileNameObj) return nullptr;
	std::string_view fileName = Python::stringAsStringView(fileNameObj);
	std::string fileNameWithExt = FilePath::withDefaultExtension(fileName, ".osc");
	ChangeWriter writer(fileNameWithExt.c_str());
	writer.write(self);
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
	// .tp_getattro = (getattrofunc)getattro,
	.tp_flags = Py_TPFLAGS_DEFAULT, // | Py_TPFLAGS_DISALLOW_INSTANTIATION,
	.tp_doc = "Changes objects",
	.tp_methods = METHODS,
	.tp_new = (newfunc)createNew,
	// .tp_richcompare = (richcmpfunc)richcompare,
};

