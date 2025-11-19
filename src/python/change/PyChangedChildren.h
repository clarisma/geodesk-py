// Copyright (c) 2025 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <Python.h>
#include <clarisma/util/TaggedPtr.h>

#include "python/util/util.h"

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

	static PyObject* append(PyChangedChildren* self, PyObject* arg);
	static PyObject* extend(PyChangedChildren* self, PyObject* arg);
	static PyObject* insert(PyChangedChildren* self, PyObject* args);
	static PyObject* remove(PyChangedChildren* self, PyObject* arg);
	static PyObject* pop(PyChangedChildren* self, PyObject* args);
	static PyObject* clear(PyChangedChildren* self, PyObject* ignored);
	static PyObject* reverse(PyChangedChildren* self, PyObject* ignored);
	static PyObject* count(PyChangedChildren* self, PyObject* arg);
	static PyObject* index(PyChangedChildren* self, PyObject* args);

	static Py_ssize_t length(PyObject* self);
	static PyObject* seqItem(PyObject* self, Py_ssize_t index);
	static int seqAssItem(PyObject* self, Py_ssize_t index, PyObject* value);
	static int contains(PyObject* self, PyObject* value);

	Changeset* changes() const { return changes_.ptr(); }
    bool containsRelationMembers() const { return changes_.flags(); }
	static PyChangedFeature* promoteChild(
        Changeset* changes, PyObject *obj, bool withRole);

private:
	PyChangedFeature* borrowAt(Py_ssize_t index) const
	{
		return (PyChangedFeature*)PyList_GET_ITEM(list_, index);
	}
	PyChangedFeature* newAt(Py_ssize_t index) const
	{
		return Python::newRef(borrowAt(index));
	}
	PyChangedFeature* accept(PyObject* obj);
  	static void childAdded(PyChangedFeature* child) {} // TODO
	static void childRemoved(PyChangedFeature* child) {} // TODO

	clarisma::TaggedPtr<Changeset,1> changes_;		// flag is true for relation
	PyObject* list_;
};
