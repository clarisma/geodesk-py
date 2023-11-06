#pragma once
#include <Python.h>
#include "feature/FeatureStore.h"

class PyTile : public PyObject
{
public:
	FeatureStore* store;
	Tile tile;
	uint32_t tip;

	static PyTypeObject TYPE;
	static PyMethodDef METHODS[];
	static PyMemberDef MEMBERS[];
	static PyGetSetDef GETSET[];
	static PyNumberMethods NUMBER_METHODS;
	static PySequenceMethods SEQUENCE_METHODS;
	static PyMappingMethods MAPPING_METHODS;

	static PyTile* create(FeatureStore* store, Tile tile, uint32_t tip);
	static void dealloc(PyTile* self);
	static PyObject* getattro(PyTile* self, PyObject *attr);
	static Py_hash_t hash(PyTile* self);
	//static PyObject* iter(PyTile* self);
	//static PyObject* next(PyTile* self);
	static PyObject* repr(PyTile* self);
	//static PyObject* richcompare(PyTile* self, PyObject* other, int op);
	static PyObject* str(PyTile* self);

	static PyObject* bounds(PyTile* self);
	static PyObject* column(PyTile* self);
	static PyObject* id(PyTile* self);
	static PyObject* revision(PyTile* self);
	static PyObject* row(PyTile* self);
	static PyObject* size(PyTile* self);
	static PyObject* zoom(PyTile* self);
};
