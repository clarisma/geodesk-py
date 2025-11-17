// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <Python.h>

class Changeset;
class PyChangedFeature;
class PyAnonymousNode;
class PyFeature;
namespace geodesk {
class FeatureStore;
}


class PyChanges : public PyObject
{
public:
	static PyTypeObject TYPE;
	static PyMethodDef METHODS[];
	static PyMappingMethods MAPPING_METHODS;

	static PyChanges* createNew(PyTypeObject* type, PyObject* args, PyObject* kwds);
	static void dealloc(PyChanges* self);
	static PyObject* getattro(PyChanges* self, PyObject *attr);
	static PyObject* getitem(PyChanges* self, PyObject* key);
	// static PyObject* richcompare(PyChanges* self, PyObject* other, int op);

	static PyObject* createFeature(PyChanges* self, PyObject* args, PyObject* kwargs);
	static PyObject* modifyFeature(PyChanges* self, PyObject* args, PyObject* kwargs);
	static PyObject* deleteFeature(PyChanges* self, PyObject* args, PyObject* kwargs);
	static PyObject* validate(PyChanges* self, PyObject* args, PyObject* kwargs);
	static PyObject* save(PyChanges* self, PyObject* args, PyObject* kwargs);

	static PyObject* str(PyChanges* self);

private:
	Changeset* changes_;
};
