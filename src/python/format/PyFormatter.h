// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <Python.h>
#include <structmember.h>

class Buffer;
class FeatureWriter;

class PyFormatter : public PyObject
{
public:
	typedef void (*WriteFunc)(PyFormatter*, Buffer*);

	PyObject* target;
	WriteFunc writeFunc;
	const char* fileExtension;
	
	static PyTypeObject TYPE;
	static PyMethodDef METHODS[];
	
	static PyObject* create(PyObject* obj, WriteFunc func, const char* ext);
	static PyObject* geojson(PyObject* obj);
	static PyObject* geojsonl(PyObject* obj);
	static PyObject* wkt(PyObject* obj);

	static PyObject* call(PyFormatter* self, PyObject* args, PyObject* kwargs);
	static void dealloc(PyFormatter* self);
	static PyObject* getattro(PyFormatter* self, PyObject *attr);
	static PyObject* repr(PyFormatter* self);
	static PyObject* str(PyFormatter* self);

	static PyObject* save(PyFormatter* self, PyObject* args);
	void write(FeatureWriter* writer);
	
	static void writeGeoJson(PyFormatter* self, Buffer* buf);
	static void writeWkt(PyFormatter* self, Buffer* buf);
};

