// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <Python.h>
#include <structmember.h>

class ChangesWeakRef;

class PyChangedMembers : public PyObject
{
public:
	PyObject_HEAD
	PyObject* list;
	ChangesWeakRef* changes;
	bool containsMembers;       // false for ways, true for relations
	bool dirty;

	static PyTypeObject TYPE;
	/*
	static PyMethodDef METHODS[];
	static PyMemberDef MEMBERS[];
	static PyGetSetDef GETSET[];
	*/
	static PySequenceMethods SEQUENCE_METHODS;
	static PyMappingMethods MAPPING_METHODS;

	static PyObject* create();
	static PyObject* call(PyChangedMembers* self, PyObject* args, PyObject* kwargs);
	static void dealloc(PyChangedMembers* self);
	static PyObject* getattro(PyChangedMembers* self, PyObject *attr);
	static Py_hash_t hash(PyChangedMembers* self);
	static PyObject* iter(PyChangedMembers* self);
	static PyObject* next(PyChangedMembers* self);
	static PyObject* repr(PyChangedMembers* self);
	static PyObject* richcompare(PyChangedMembers* self, PyObject* other, int op);
	static PyObject* str(PyChangedMembers* self);
};
