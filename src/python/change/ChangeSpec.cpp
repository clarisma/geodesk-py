// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include <execution>

#include "ChangeSpec.h"
#include "Changeset.h"
#include "FeatureBuilder.h"
#include "PyChangedFeature.h"
#include "python/Environment.h"
#include "python/feature/PyFeature.h"
#include "python/geom/PyCoordinate.h"
#include "python/geom/PyMercator.h"

// TODO: use mapping protocol instead of PyDict_ functions
//  so user can assign PyTags to tags
//  but careful, PyFeature is also a map-like object

// TODO: Ensure args stays referenced for lifetime of this object
//  (we use borrowed refs for tag keys/values)
bool ChangeSpec::parse(PyObject* args, int start, PyObject* kwargs)
{
	assert(PyTuple_Check(args));
    Py_ssize_t argCount = PyTuple_Size(args);
    for (Py_ssize_t i=start; i<argCount; i++)
    {
        PyObject* arg = PyTuple_GET_ITEM(args, i); // borrowed ref
		if (Py_TYPE(arg) == &PyCoordinate::TYPE)
        {
            auto* coord = reinterpret_cast<PyCoordinate*>(arg);
			if (!acceptShapeType(COORDINATE)) return false;
		    coordinate_ = FixedLonLat(coord->coordinate());
			continue;
        }

    	if (PyUnicode_Check(arg))		// String (key of tag)
    	{
    		PyObject* key = arg;
    		i++;
    		if (i >= argCount)
    		{
    			ChangedTags::errorExpectedTag();
    			return false;
    		}
    		PyObject* value = PyTuple_GET_ITEM(args, i); // borrowed ref
			if (!tags_.addTag(key, value)) return false;
    		continue;
    	}

    	if (PyDict_Check(arg))  // dictionary of tags
    	{
    		if (!tags_.addFromDict(arg)) return false;
    		continue;
    	}

    	if (PySequence_Check(arg))  // tags, nodes or members
    	{
    		PyObject *seq = PySequence_Fast(arg, "Expected a sequence");
    		if (!seq) return false;		// should never happen
    		if (!acceptSequenceArg(seq))
			{
				Py_DECREF(seq);
				return false;
			}
    		Py_DECREF(seq);
    		continue;
    	}

    	if (PyNumber_Check(arg))
    	{
    		PyObject* first = arg;
    		i++;
    		if (i >= argCount)
    		{
    			PyErr_SetString(PyExc_TypeError, "Expected coordinate pair");
    			return false;
    		}
    		PyObject* second = PyTuple_GET_ITEM(args, i); // borrowed ref
    		if (!acceptCoordinate(first, second)) return false;
    		continue;
    	}

    	GEOSGeometry* geom;
    	if (Environment::get().getGeosGeometry(arg, &geom))
    	{
    		if (!acceptShapeType(GEOMETRY)) return false;
    		geom_ = geom;
			continue;
    	}

    	PyErr_Format(PyExc_TypeError, "Invalid argument of type %s",
    		Py_TYPE(arg)->tp_name);
    	return false;
    }

	// TODO: keyword args


    return true;
}

void ChangeSpec::errorExpectedTag()
{
	PyErr_SetString(PyExc_TypeError, "Expected a key/value pair");
}

bool ChangeSpec::acceptSequenceArg(PyObject* seq)
{
	Py_ssize_t n = PySequence_Fast_GET_SIZE(seq);
	if (n > 0)
	{
		PyObject* first = PySequence_Fast_GET_ITEM(seq, 0);
		if (PyUnicode_Check(first))
		{
			return tags_.addFromSequence(seq);
		}
		if (PyNumber_Check(first))
		{
			if (n != 2)
			{
				PyErr_SetString(PyExc_TypeError, "Expected a coordinate");
				return false;
			}
			return acceptCoordinate(first, PySequence_Fast_GET_ITEM(seq, 1));
		}
		if (PySequence_Check(first))
		{
			PyObject *childSeq = PySequence_Fast(first,
				"Expected a sequence");
			if (!childSeq) return false;
			Py_ssize_t tupleSize = PySequence_Fast_GET_SIZE(childSeq);
			if (tupleSize == 2 && PyUnicode_Check(
				PySequence_Fast_GET_ITEM(childSeq, 0)))
			{
				// TODO: should we allow (<role>,<feature>) ?
				//  If so, must disambiguate from tag
				Py_DECREF(childSeq);
				return tags_.addFromSequence(seq);
			}
			Py_DECREF(childSeq);
		}
		return acceptChildSequence(seq);
	}
	return true;
}


bool ChangeSpec::acceptCoordinate(PyObject* first, PyObject* second)
{
	if (!PyMercator::getAgnosticLonLat(&coordinate_, first, second))
	{
		return false;
	}
	return acceptShapeType(COORDINATE);
}

PyChangedFeature* ChangeSpec::create()
{
	PyChangedFeature* feature;

	if (received_ == GEOMETRY)
	{
		assert(geom_);
		GEOSContextHandle_t context = Environment::get().getGeosContext();
		if (!context) return nullptr;
		FeatureBuilder builder(changes_, context);
		feature = builder.fromGeometry(geom_);
	}
	else if (received_ == COORDINATE)
	{
		feature = changes_->createNode(coordinate_);
	}
	else if (received_ == NODES)
	{
		feature = changes_->createFeature2D(
			PyChangedFeature::Type::WAY, children_.release());
	}
	else if (received_ == MEMBERS)
	{
		feature = changes_->createFeature2D(
			PyChangedFeature::Type::RELATION, children_.release());
	}
	else
	{
		PyErr_SetString(PyExc_TypeError,
			"Expected Geometry, coordinate, nodes or members");
		return nullptr;
	}

	if (!changeTags(feature))
	{
		Py_DECREF(feature);
		return nullptr;
	}
	return feature;
}


bool ChangeSpec::modify(PyChangedFeature* feature) const
{
	// TODO: set coord, nodes, members

	if (!changeTags(feature)) return false;
	return true;
}


bool ChangeSpec::changeTags(PyChangedFeature* feature) const
{
	if (!tags_.isEmpty())
	{
		PyObject* tags = feature->tags();
		if (!tags) return false;
		return tags_.applyTo(tags);
	}
	return true;
}

bool ChangeSpec::acceptShapeType(int shapeType)
{
	if ((accept_ & shapeType) == 0)
	{
		PyErr_Format(PyExc_ValueError, "%s not accepted",
			shapeTypeName(shapeType));
		return false;
	}
	if (received_)
	{
		PyErr_Format(PyExc_ValueError, "Can't accept %s (already supplied %s)",
			shapeTypeName(shapeType), shapeTypeName(received_));
		return false;
	}
	received_ = shapeType;
	return true;
}

const char* ChangeSpec::shapeTypeName(int shapeType)
{
	switch (shapeType)
	{
	case GEOMETRY:		return "Geometry";
	case COORDINATE:	return "coordinate";
	case NODES:			return "nodes";
	case MEMBERS:		return "members";
	default:
		assert(false);
		return "";
	}
}

bool ChangeSpec::acceptChildSequence(PyObject* seq)
{
	PyObject* children = PyChangedFeature::createChildren(
		changes_, seq, (accept_ & NODES) == 0);
	if (!children) return false;
	if (PyList_GET_SIZE(children) > 0)
	{
		bool containsMembers =
			((PyChangedFeature*)PyList_GET_ITEM(
				children, 0))->isMember();
		if (!acceptShapeType(containsMembers ? MEMBERS : NODES))
		{
			return false;
		}
		children_.reset(children);
	}
	return true;
}