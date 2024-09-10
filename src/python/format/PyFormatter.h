// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <Python.h>
#include <structmember.h>
#include "feature/FeaturePtr.h"

class Buffer;
class FeatureStore;
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

	PyObject* idSchema;
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
	static int setattro(PyFormatter* self, PyObject* attr, PyObject* value);
	static PyObject* repr(PyFormatter* self);
	static PyObject* str(PyFormatter* self);

	int lookupAttr(PyObject* key);
	int setAttribute(PyObject* attr, PyObject* value);
	int setAttributes(PyObject* dict);
	int setId(PyObject* value);

	static PyObject* save(PyFormatter* self, PyObject* args, PyObject* kwargs);
	void write(FeatureWriter* writer);
	
	static void writeIdViaCallable(FeatureWriter* writer,
		FeatureStore* store, FeaturePtr feature, /* PyObject */ void* closure);

	static void writeGeoJson(PyFormatter* self, Buffer* buf);
	static void writeWkt(PyFormatter* self, Buffer* buf);
};

