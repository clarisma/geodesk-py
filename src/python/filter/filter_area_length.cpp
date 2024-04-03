// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "filters.h"
#include <limits>
#include "filter/AreaFilter.h"
#include "filter/LengthFilter.h"
#include "geom/LengthUnit.h"


double getUnit(PyObject* args, PyObject* kwargs, bool squared)
{
	Py_ssize_t argCount = PySequence_Length(args);
	if (kwargs)
	{
		Py_ssize_t kwCount = PyDict_Size(kwargs);
		if (argCount > 0 || kwCount > 1)
		{
			PyErr_SetString(PyExc_TypeError, "Too many arguments");
			return -1.0;
		}
		PyObject* key;
		PyObject* value;
		Py_ssize_t pos = 0;
		if (!PyDict_Next(kwargs, &pos, &key, &value))
		{
			PyErr_SetString(PyExc_TypeError, "Missing arguments");
			return -1.0;
		}
		Py_ssize_t keyLen;
		const char* keyStr = PyUnicode_AsUTF8AndSize(key, &keyLen);
		if (!keyStr) return -1.0;

		int unit = LengthUnit::unitFromString(std::string_view(keyStr, keyLen));
		if (unit < 0)
		{
			PyErr_SetString(PyExc_TypeError, "Invalid unit");
			return -1.0;
		}
		double val = PyFloat_AsDouble(value);
		if (val == -1.0 && PyErr_Occurred()) return val;

		double ratio = LengthUnit::UNITS_TO_METERS[unit];
		if (squared) ratio *= ratio;
		return val * ratio;
	}

	if (argCount != 1)
	{
		PyErr_SetString(PyExc_TypeError, argCount ?
			"Too many arguments" : "Missing arguments");
		return -1.0;
	}
	return PyFloat_AsDouble(PyTuple_GET_ITEM(args, 0));
}


PyFeatures* filters::min_area(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	double a = getUnit(args, kwargs, true);
	if (a > 0)
	{
		return self->withFilter(new AreaFilter(a, std::numeric_limits<double>::max()));
	}
	if (PyErr_Occurred()) return NULL;
	return Python::newRef(self);
}


PyFeatures* filters::max_area(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	double a = getUnit(args, kwargs, true);
	if (a >= 0)
	{
		return self->withFilter(new AreaFilter(std::numeric_limits<double>::min(), a));
	}
	if (PyErr_Occurred()) return NULL;
	return Python::newRef(self);
	return self->getEmpty();
}


PyFeatures* filters::min_length(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	double a = getUnit(args, kwargs, false);
	if (a > 0)
	{
		return self->withFilter(new LengthFilter(a, std::numeric_limits<double>::max()));
	}
	if (PyErr_Occurred()) return NULL;
	return Python::newRef(self);
}


PyFeatures* filters::max_length(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	double a = getUnit(args, kwargs, false);
	if (a >= 0)
	{
		return self->withFilter(new LengthFilter(std::numeric_limits<double>::min(), a));
	}
	if (PyErr_Occurred()) return NULL;
	return Python::newRef(self);
	return self->getEmpty();
}
