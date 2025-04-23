// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <Python.h>
#include <clarisma/util/TaggedPtr.h>

class ChangesWeakRef;
class PyChanges;
class PyFeature;

class PyChangedMembers : public PyObject
{
public:
	PyObject_HEAD
	clarisma::TaggedPtr<ChangesWeakRef,1> changes;		// flag is true for relation
	PyObject* list;

	static PyTypeObject TYPE;
	/*
	static PyMethodDef METHODS[];
	static PyMemberDef MEMBERS[];
	static PyGetSetDef GETSET[];
	*/
	static PySequenceMethods SEQUENCE_METHODS;
	static PyMappingMethods MAPPING_METHODS;

	static PyChangedMembers* create(PyChanges* changes, bool forRelation);
	static PyChangedMembers* create(PyChanges* changes, PyObject* list, bool forRelation);
	static PyChangedMembers* create(PyChanges* changes, PyFeature* parent);
	static void dealloc(PyChangedMembers* self);
	static PyObject* getattro(PyChangedMembers* self, PyObject *attr);
	static PyObject* iter(PyChangedMembers* self);
	static PyObject* repr(PyChangedMembers* self);
	static PyObject* richcompare(PyChangedMembers* self, PyObject* other, int op);
	static PyObject* str(PyChangedMembers* self);

	bool containsRelationMembers() const { return changes.flags(); }
};
