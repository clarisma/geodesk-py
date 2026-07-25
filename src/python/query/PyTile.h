// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <Python.h>
#include <geodesk/feature/FeatureStore.h>
#include <geodesk/feature/Tip.h>
#include <geodesk/geom/Tile.h>

using namespace geodesk;

class PyTile : public PyObject
{
public:
	FeatureStore* store;
	Tile tile_;
	Tip tip_;

	static PyTypeObject TYPE;
	static PyMethodDef METHODS[];
	static PyMemberDef MEMBERS[];
	static PyGetSetDef GETSET[];
	static PyNumberMethods NUMBER_METHODS;
	static PySequenceMethods SEQUENCE_METHODS;
	static PyMappingMethods MAPPING_METHODS;

	static PyTile* create(FeatureStore* store, Tile tile, Tip tip);
	static void dealloc(PyTile* self);
	static PyObject* getattro(PyTile* self, PyObject *attr);
	static Py_hash_t hash(PyTile* self);
	//static PyObject* iter(PyTile* self);
	//static PyObject* next(PyTile* self);
	static PyObject* repr(PyTile* self);
	static PyObject* richcompare(PyTile* self, PyObject* other, int op);
	static PyObject* str(PyTile* self);

	static PyObject* bounds(PyTile* self);
	static PyObject* children(PyTile* self);
	static PyObject* column(PyTile* self);
	static PyObject* exports(PyTile* self);
	static PyObject* features(PyTile* self);
	static PyObject* id(PyTile* self);
	static PyObject* indexes(PyTile* self);
	static PyObject* is_active(PyTile* self);
	static PyObject* is_current(PyTile* self);
	static PyObject* is_loaded(PyTile* self);
	static PyObject* parent(PyTile* self);
	static PyObject* revision(PyTile* self);
	static PyObject* row(PyTile* self);
	static PyObject* shape(PyTile* self);
	static PyObject* size(PyTile* self);
	static PyObject* tip(PyTile* self);
	static PyObject* zoom(PyTile* self);
};
