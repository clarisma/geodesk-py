// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <Python.h>
#include <clarisma/util/TaggedPtr.h>

class Changeset;
class PyChangedFeature;
class PyFeature;

class PyChangedMembers : public PyObject
{
public:
	clarisma::TaggedPtr<Changeset,1> changes;		// flag is true for relation
	PyObject* list;

	static PyTypeObject TYPE;
	/*
	static PyMethodDef METHODS[];
	static PyMemberDef MEMBERS[];
	static PyGetSetDef GETSET[];
	*/
	static PySequenceMethods SEQUENCE_METHODS;
	static PyMappingMethods MAPPING_METHODS;

	static PyChangedMembers* create(Changeset* changes, bool forRelation);
	static PyChangedMembers* create(Changeset* changes, PyObject* list, bool forRelation);
	static PyChangedMembers* create(Changeset* changes, PyFeature* parent);
	static PyChangedMembers* fromSequence(Changeset* changes, PyObject* seq, bool forRelation);
	static void dealloc(PyChangedMembers* self);
	static PyObject* getattro(PyChangedMembers* self, PyObject *attr);
	static PyObject* getitem(PyChangedMembers* self, PyObject* key);
	static int setitem(PyChangedMembers* self, PyObject* key, PyObject* value);
	static PyObject* iter(PyChangedMembers* self);
	static PyObject* repr(PyChangedMembers* self);
	static PyObject* richcompare(PyChangedMembers* self, PyObject* other, int op);
	static PyObject* str(PyChangedMembers* self);

	bool containsRelationMembers() const { return changes.flags(); }

private:
	static bool tryAcceptMember(PyChangedFeature** changed, Changeset* changes, PyObject* obj);
	static PyChangedFeature* acceptMember(Changeset* changes, PyObject* obj);
};
