// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <Python.h>
#include <structmember.h>

class Buffer;
class FeatureWriter;

class PyFormatter : public PyObject
{
public:
	enum Attribute
	{
		EXCLUDE_KEYS,
		ID,
		KEYS,
		LIMIT,
		LINEWISE,
		MAX_WIDTH,
		MERCATOR,
		PRECISION,
		PRETTY,
		SAVE,			// method
		SCALE,
		SIMPLIFY,
		SORT_TAGS,
		TRANSLATE,
	};

	int64_t limit;
	double scale;
	double translateX;
	double translateY;
	int precision;
	bool pretty;
	bool linewise;
	bool mercator;
	bool sortTags;
	

	typedef void (*WriteFunc)(PyFormatter*, Buffer*);

	PyObject* target;
	WriteFunc writeFunc;
	const char* fileExtension;
	
	static PyTypeObject TYPE;
	static PyMethodDef METHODS[];
	
	static PyFormatter* create(PyObject* obj, WriteFunc func, const char* ext);
	static PyObject* geojson(PyObject* obj);
	static PyObject* geojsonl(PyObject* obj);
	static PyObject* wkt(PyObject* obj);

	static PyObject* call(PyFormatter* self, PyObject* args, PyObject* kwargs);
	static void dealloc(PyFormatter* self);
	static PyObject* getattro(PyFormatter* self, PyObject *attr);
	static PyObject* repr(PyFormatter* self);
	static PyObject* str(PyFormatter* self);

	int lookupAttr(PyObject* key);
	int setAttribute(PyObject* attr, PyObject* value);
	int setAttributes(PyObject* dict);

	static PyObject* save(PyFormatter* self, PyObject* args, PyObject* kwargs);
	void write(FeatureWriter* writer);
	
	static void writeGeoJson(PyFormatter* self, Buffer* buf);
	static void writeWkt(PyFormatter* self, Buffer* buf);
};

