// Copyright (c) 2025 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <Python.h>

class PyListProxy : public PyObject
{
public:
	using CoerceFunc = PyObject* (*)(PyObject* context, PyObject* item);
	using EqualsFunc = bool (*)(PyObject* item, PyObject* other);
	using ItemFunc = void (*)(PyObject* context, PyObject* list, PyObject* item);
	using ListFunc = void (*)(PyObject* context, PyObject* list);

	struct Operations
	{
		CoerceFunc coerceItem;
		EqualsFunc itemEquals;
		ItemFunc itemAdded;
		ItemFunc itemRemoved;
		ListFunc listReordered;
	};
	static PyTypeObject TYPE;

    static PyMethodDef METHODS[];
	static PySequenceMethods SEQUENCE_METHODS;
	static PyMappingMethods MAPPING_METHODS;

	static PyListProxy* create(PyObject* list, PyObject* context, const Operations* ops);
    static void dealloc(PyObject* self);
    static PyObject* iter(PyObject* self);
	static PyObject* repr(PyObject* self);
	static PyObject* richcompare(PyObject* self, PyObject* other, int op);
	static PyObject* str(PyObject* self);

	static PyObject* append(PyObject* list, PyObject* arg,
		PyObject* context, CoerceFunc coerce, ItemFunc added);
	static PyObject* extend(PyObject* list, PyObject* arg,
		PyObject* context, CoerceFunc coerce, ItemFunc added);
	static PyObject* insert(PyObject* list, PyObject* args,
		PyObject* context, CoerceFunc coerce, ItemFunc added);
	static PyObject* remove(PyObject* list, PyObject* arg,
		PyObject* context, EqualsFunc equals, ItemFunc removed);
	static PyObject* removeAll(PyObject* list, PyObject* arg,
		PyObject* context, EqualsFunc equals, ItemFunc removed);
	static int removeByKey(PyObject* list, PyObject* key,
		PyObject* context, ItemFunc removed);
	static PyObject* pop(PyObject* list, PyObject* args,
		PyObject* context, ItemFunc removed);
	static PyObject* clear(PyObject* list,
		PyObject* context, ItemFunc removed);
	static PyObject* reverse(PyObject* list,
		PyObject* context, ListFunc reordered);

	static PyObject* count(PyObject* list, PyObject* arg, EqualsFunc equals);
	static PyObject* index(PyObject* list, PyObject* args, EqualsFunc equals);
	static int contains(PyObject* list, PyObject* value, EqualsFunc equals);
	static Py_ssize_t length(PyObject* self);
	static PyObject* seqItem(PyObject* self, Py_ssize_t index);
	static int seqAssItem(PyObject* self, Py_ssize_t index, PyObject* value);

private:
	static PyObject* getitem(PyObject* self, PyObject* key);
	static int setitem(PyObject* self, PyObject* key, PyObject* value);

	static PyObject* _append(PyObject* self, PyObject* arg);
	static PyObject* _extend(PyObject* self, PyObject* arg);
	static PyObject* _insert(PyObject* self, PyObject* args);
	static PyObject* _remove(PyObject* self, PyObject* arg);
	static PyObject* _remove_all(PyObject* self, PyObject* arg);
	static PyObject* _pop(PyObject* self, PyObject* args);
	static PyObject* _clear(PyObject* self, PyObject* ignored);
	static PyObject* _reverse(PyObject* self, PyObject* ignored);
	static PyObject* _count(PyObject* self, PyObject* arg);
	static PyObject* _index(PyObject* self, PyObject* args);
	static int _contains(PyObject* list, PyObject* value);

	static PyObject* listFromIterable(PyObject* iterable,
		PyObject* context, CoerceFunc coerce);

	PyObject* list_;
	PyObject* context_;
	const Operations* ops_;
};
