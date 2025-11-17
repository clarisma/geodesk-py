// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyChangedFeature.h"
#include "PyChanges.h"
#include "python/geom/PyCoordinate.h"

bool PyChangedFeature::Parameters::parse(PyObject* args, int start, PyObject* kwargs)
{
	assert(PyTuple_Check(args));
    Py_ssize_t argCount = PyTuple_Size(args);
    for (Py_ssize_t i=start; i<argCount; i++)
    {
        PyObject* arg = PyTuple_GET_ITEM(args, i); // borrowed ref
		if (Py_TYPE(arg) == &PyCoordinate::TYPE)
        {
            auto* coord = reinterpret_cast<PyCoordinate*>(arg);
		    if (received_ & COORDINATE)
		    {
                // TODO: duplicate coord
		    }
		    received_ |= COORDINATE;
			coordinate_ = FixedLonLat(coord->coordinate());
			continue;
        }

    	if (PyUnicode_Check(arg))		// String (key of tag)
    	{
    		PyObject* key = arg;
    		i++;
    		if (i >= argCount)
    		{
    			// TODO: error: missing tag value
    		}
    		PyObject* value = PyTuple_GET_ITEM(args, i); // borrowed ref
			if (!acceptTag(key, value)) return false;
    		continue;
    	}

    	if (PyTuple_Check(arg))		// coord pair or tag
    	{
    		Py_ssize_t tupleSize = PyTuple_Size(arg);
    		if (tupleSize != 2)
    		{
    			// TODO: Error: Expected str/value or num/num
    		}

    		PyObject* first = PyTuple_GET_ITEM(arg, 0);
    		PyObject* second = PyTuple_GET_ITEM(arg, 1);

    		if (PyUnicode_Check(first))		// String (key of tag)
    		{
    			if (!acceptTag(first, second)) return false;
    			continue;
    		}

    		// TODO: process coordinate
    		continue;
    	}

    	if (PyDict_Check(arg))  // dictionary of tags
    	{
    		// TODO
    		continue;
    	}

    	if (PyList_Check(arg))  // tags, nodes or members
    	{
    		// TODO
    		continue;
    	}

    	GEOSGeometry* geom;
    	if (Environment::get().getGeosGeometry(value, &geom))
    	{
    		// TODO
    	}
    }

	// TODO: keyword args


    return true;
}

bool PyChangedFeature::Parameters::acceptTag(PyObject* key, PyObject* value)
{
	if (value == Py_None ||
		(PyUnicode_Check(value) && PyUnicode_GET_LENGTH(value) == 0))
	{
		deletedKeys_.emplace_back(PyObjectRef::makeNew(key));
		return true;
	}
	modifiedTags_.emplace_back(
		PyObjectRef::makeNew(key),
		PyObjectRef::makeNew(value));
	return true;
}


PyChangedFeature* PyChangedFeature::Parameters::create()
{
	if (received_ == 0)
	{
		// TODO: Error: Expected coord,geom,nodes or members
		return nullptr;
	}

	if (received_ == GEOMETRY)
	{
		// TODO: build from geometry
	}

	if (received_ == COORDINATE)
	{
		// TODO: build from geometry
	}

}