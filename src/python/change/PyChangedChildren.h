// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <Python.h>
#include <clarisma/util/TaggedPtr.h>

class Changeset;
class PyChangedFeature;
class PyFeature;

class PyChangedChildren : public PyObject
{
public:
	static PyTypeObject TYPE;

    static PyMethodDef METHODS[];
	static PySequenceMethods SEQUENCE_METHODS;
	static PyMappingMethods MAPPING_METHODS;

	static PyChangedChildren* create(Changeset* changes, bool forRelation);
	static PyChangedChildren* create(Changeset* changes, PyObject* list, bool forRelation);
	static PyChangedChildren* create(Changeset* changes, PyFeature* parent);
	static PyChangedChildren* fromSequence(Changeset* changes, PyObject* seq, bool forRelation);
    static void dealloc(PyChangedChildren* self);
    static PyObject* getitem(PyChangedChildren* self, PyObject* key);
	static int setitem(PyChangedChildren* self, PyObject* key, PyObject* value);
	static PyObject* iter(PyChangedChildren* self);
	static PyObject* repr(PyChangedChildren* self);
	static PyObject* richcompare(PyChangedChildren* self, PyObject* other, int op);
	static PyObject* str(PyChangedChildren* self);

	Changeset* changes() const { return changes_.ptr(); }
    bool containsRelationMembers() const { return changes_.flags(); }
	static PyChangedFeature* promoteChild(
        Changeset* changes, PyObject *obj, bool withRole);

private:
  	void childAdded(PyChangedFeature* child);
	void childRemoved(PyChangedFeature* child);

	clarisma::TaggedPtr<Changeset,1> changes_;		// flag is true for relation
	PyObject* list_;
};
