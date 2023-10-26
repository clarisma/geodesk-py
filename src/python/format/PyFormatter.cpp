// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyFormatter.h"
#include <common/util/Buffer.h>
#include "format/FeatureWriter.h"
#include "format/GeoJsonWriter.h"
#include "format/WktWriter.h"
#include "python/feature/PyFeature.h"
#include "python/util/util.h"

PyObject* PyFormatter::create(PyObject* obj, WriteFunc func, const char* ext)
{
	PyFormatter* self = (PyFormatter*)TYPE.tp_alloc(&TYPE, 0);
	if (self)
	{
		self->target = Python::newRef(obj);
		self->writeFunc = func;
		self->fileExtension = ext;
	}
	return self;
}

PyObject* PyFormatter::call(PyFormatter* self, PyObject* args, PyObject* kwargs)
{
	// TODO
	Py_RETURN_NONE;
}

void PyFormatter::dealloc(PyFormatter* self)
{
	Py_DECREF(self->target);
}

PyObject* PyFormatter::getattro(PyFormatter* self, PyObject* attr)
{
	// TODO
	return PyObject_GenericGetAttr(self, attr);
}


PyObject* PyFormatter::repr(PyFormatter* self)
{
	// TODO
	Py_RETURN_NONE;
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

PyObject* PyFormatter::save(PyFormatter* self, PyObject* args)
{
	PyObject* arg = Python::checkSingleArg(args, NULL, "<filename>");
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

PyMethodDef PyFormatter::METHODS[] =
{
	{"save", (PyCFunction)save, METH_VARARGS, "Saves the file" },
	{ NULL, NULL, 0, NULL },
};


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
	.tp_methods = METHODS,
};


void PyFormatter::writeGeoJson(PyFormatter* self, Buffer* buf)
{
	GeoJsonWriter writer(buf);
	self->write(&writer);
}

PyObject* PyFormatter::geojson(PyObject* obj)
{
	return create(obj, &writeGeoJson, ".geojson");
}

PyObject* PyFormatter::geojsonl(PyObject* obj)
{
	PyErr_SetString(PyExc_NotImplementedError,
		"This feature will be available in Version 0.2.0");
	return NULL;
}

void PyFormatter::writeWkt(PyFormatter* self, Buffer* buf)
{
	WktWriter writer(buf);
	self->write(&writer);
}

PyObject* PyFormatter::wkt(PyObject* obj)
{
	return create(obj, &writeWkt, ".wkt");
}
