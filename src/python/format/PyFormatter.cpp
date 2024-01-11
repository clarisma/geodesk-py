// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyFormatter.h"
#include <common/util/Buffer.h>
#include "format/FeatureWriter.h"
#include "format/GeoJsonWriter.h"
#include "format/WktWriter.h"
#include "python/feature/PyFeature.h"
#include "python/util/PyFastMethod.h"
#include "python/util/util.h"

#include "PyFormatter_attr.cxx"
#include "PyFormatter_lookup.cxx"


PyFormatter* PyFormatter::create(PyObject* obj, WriteFunc func, const char* ext)
{
	PyFormatter* self = (PyFormatter*)TYPE.tp_alloc(&TYPE, 0);
	if (self)
	{
		self->target = Python::newRef(obj);
		self->writeFunc = func;
		self->fileExtension = ext;
		self->limit = INT64_MAX;
		self->scale = 1.0;
		self->translateX = 0.0;
		self->translateY = 0.0;
		self->precision = 7;
		self->pretty = false;
		self->linewise = false;
		self->mercator = false;
		self->sortTags = false;
	}
	return self;
}

int PyFormatter::setAttributes(PyObject* dict)
{
	PyObject* key;
	PyObject* value;
	Py_ssize_t pos = 0;

	while (PyDict_Next(dict, &pos, &key, &value))
	{
		if (setAttribute(key, value) < 0) return -1;
	}
	return 0;
}

PyObject* PyFormatter::call(PyFormatter* self, PyObject* args, PyObject* kwargs)
{
	// TODO: Shouldn't we create a copy of the Formatter?
	if (kwargs)
	{
		if (self->setAttributes(kwargs) < 0) return NULL;
	}
	return Python::newRef(self);
}

void PyFormatter::dealloc(PyFormatter* self)
{
	Py_DECREF(self->target);
}

int PyFormatter::lookupAttr(PyObject* key)
{
	Py_ssize_t len;
	const char* name = PyUnicode_AsUTF8AndSize(key, &len);
	if (!name) return -1;
	// TODO: distinguish between error and unknown attr

	Attr* attr = PyFormatter_AttrHash::lookup(name, len);
	if (!attr) return -1;
	return attr->index;
}

PyObject* PyFormatter::getattro(PyFormatter* self, PyObject* attr)
{
	int index = self->lookupAttr(attr);
	switch (index)
	{
	case LIMIT:
		return PyLong_FromLong(self->limit);
	case LINEWISE:
		return Python::boolValue(self->linewise);
	case MERCATOR:
		return Python::boolValue(self->mercator);
	case PRECISION:
		return PyLong_FromLong(self->precision);
	case PRETTY:
		return Python::boolValue(self->pretty);
	case SAVE:
		return PyFastMethod::create(self, (PyCFunctionWithKeywords)&save);
	}
	return PyObject_GenericGetAttr(self, attr);
}

int PyFormatter::setAttribute(PyObject* attr, PyObject* value)
{
	int index = lookupAttr(attr);
	switch (index)
	{
	case LIMIT:
		if (value == Py_None)
		{
			limit = INT64_MAX;
			return 0;
		}
		return Python::setLong(value, &limit, 1, INT64_MAX);
	case LINEWISE:
		return Python::setBool(value, &linewise);
	case MERCATOR:
		return Python::setBool(value, &mercator);
	case PRECISION:
		return Python::setInt(value, &precision, 0, 15);
	case PRETTY:
		return Python::setBool(value, &pretty);
	}
	PyErr_SetObject(PyExc_AttributeError, attr);
	return -1;
}



PyObject* PyFormatter::repr(PyFormatter* self)
{
	return str(self);
}

PyObject* PyFormatter::str(PyFormatter* self)
{
	DynamicBuffer buf(64 * 1024);
	self->writeFunc(self, &buf);
	return PyUnicode_FromStringAndSize(buf.data(), buf.length());
}

void PyFormatter::write(FeatureWriter* writer)
{
	PyTypeObject* type = Py_TYPE(target);
	if (type == &PyFeature::TYPE)
	{
		PyFeature* feature = (PyFeature*)target;
		writer->writeFeature(feature->store, feature->feature);
	}	// TODO: AnonymousNode
	else if (Python::isIterable(target))
	{
		writer->writeHeader();
		PyObject* iter = PyObject_GetIter(target);
		PyObject* item;
		while ((item = PyIter_Next(iter)))
		{
			PyTypeObject* childType = Py_TYPE(item);
			if (Py_TYPE(item) == &PyFeature::TYPE)
			{
				PyFeature* feature = (PyFeature*)item;
				writer->writeFeature(feature->store, feature->feature);
			}
			// TODO: AnonymousNode
			Py_DECREF(item);
		}
		writer->writeFooter();
	}
	writer->flush();
}

PyObject* PyFormatter::save(PyFormatter* self, PyObject* args, PyObject* kwargs)
{
	PyObject* arg = Python::checkSingleArg(args, kwargs, "<filename>");
	if (arg == NULL) return NULL;
	const char* fileName = PyUnicode_AsUTF8(arg);
	if (!fileName) return NULL;
	std::string s;
	if (File::extension(fileName)[0] == 0)
	{
		s = std::string(fileName) + self->fileExtension;
		fileName = s.c_str();
	}
	FILE* file = fopen(fileName, "wb");
	if (!file)
	{
		PyErr_Format(PyExc_IOError, "Failed to open %s for writing", fileName);
		return NULL;
	}
	FileBuffer buf(file, 64 * 1024);
	self->writeFunc(self, &buf);
	// no need to close file, ~FileBuffer does this
	Py_RETURN_NONE;
}

/*
PyMethodDef PyFormatter::METHODS[] =
{
	{"save", (PyCFunction)save, METH_VARARGS, "Saves the file" },
	{ NULL, NULL, 0, NULL },
};
*/

PyTypeObject PyFormatter::TYPE =
{
	.tp_name = "geodesk.Formatter",
	.tp_basicsize = sizeof(PyFormatter),
	.tp_dealloc = (destructor)dealloc,
	.tp_repr = (reprfunc)repr,
	.tp_call = (ternaryfunc)call,
	.tp_str = (reprfunc)str,
	.tp_getattro = (getattrofunc)getattro,
	.tp_flags = Py_TPFLAGS_DEFAULT, // | Py_TPFLAGS_DISALLOW_INSTANTIATION,
	.tp_doc = "Formatter objects",
	// .tp_methods = METHODS,
};


void PyFormatter::writeGeoJson(PyFormatter* self, Buffer* buf)
{
	GeoJsonWriter writer(buf);
	writer.precision(self->precision);
	writer.pretty(self->pretty);
	self->write(&writer);
}

PyObject* PyFormatter::geojson(PyObject* obj)
{
	return create(obj, &writeGeoJson, ".geojson");
}

PyObject* PyFormatter::geojsonl(PyObject* obj)
{
	PyFormatter* formatter = create(obj, &writeGeoJson, ".geojsonl");
	formatter->linewise = true;
	return formatter;
}

void PyFormatter::writeWkt(PyFormatter* self, Buffer* buf)
{
	WktWriter writer(buf);
	writer.precision(self->precision);
	writer.pretty(self->pretty);
	self->write(&writer);
}

PyObject* PyFormatter::wkt(PyObject* obj)
{
	return create(obj, &writeWkt, ".wkt");
}

