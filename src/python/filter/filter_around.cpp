// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "filters.h"
#include "feature/Node.h"
#include "filter/PointDistanceFilter.h"
#include "geom/Centroid.h"
#include "geom/LengthUnit.h"
#include "python/feature/PyFeature.h"
#include "python/geom/PyCoordinate.h"
#include "python/util/util.h"
#include <geos/geom/Coordinate.h>
#include <geos/geom/Geometry.h>
#include <geos/algorithm/Centroid.h>


enum ArgFlags
{
	HAS_X = 1,
	HAS_Y = 2,
	HAS_DISTANCE = 4
};

PyFeatures* filters::around(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	int argFlags = 0;
	double distance = 0;
	::Coordinate point(0,0);
	Py_ssize_t argCount = PySequence_Length(args);
	if (argCount == 1)
	{
		PyObject* arg = PyTuple_GET_ITEM(args, 0);
		PyTypeObject* type = Py_TYPE(arg);
		if (type == &PyFeature::TYPE)
		{
			PyFeature* feature = (PyFeature*)arg;
			point = Centroid::ofFeature(feature->store, feature->feature);
		}
		else if (type == &PyCoordinate::TYPE)
		{
			point = ((PyCoordinate*)arg)->coordinate();
		}
		else
		{
			geos::geom::Geometry* geom;
			if (Environment::get().getGeosGeometry(arg, (GEOSGeometry**)&geom))
			{
				geos::geom::Coordinate coord;
				if (!geos::algorithm::Centroid::getCentroid(*geom, coord))
				{
					PyErr_SetString(PyExc_RuntimeError, "Centroid calculation failed.");
					return NULL;
				}
				point = ::Coordinate(coord.x, coord.y);
			}
			else
			{
				PyErr_Clear();
				PyErr_Format(PyExc_TypeError, 
					"Expected geometric object (instead of %s)", type->tp_name);
				return NULL;
			}
		}
		argFlags |= ArgFlags::HAS_X | ArgFlags::HAS_Y;
	}

	if (kwargs)
	{
		PyObject* key;
		PyObject* value;
		Py_ssize_t pos = 0;
		while (PyDict_Next(kwargs, &pos, &key, &value))
		{
			// References are borrowed, don't dec refcount
			Py_ssize_t len;
			const char* keyStr = PyUnicode_AsUTF8AndSize(key, &len);
			if (!keyStr) return NULL;
			int unit = LengthUnit::unitFromString(std::string_view(keyStr, len));
			if (unit >= 0)
			{
				distance = PyFloat_AsDouble(value);
				if (distance == -1.0 && PyErr_Occurred()) return NULL;
				distance = LengthUnit::toMeters(distance, unit);
				argFlags |= ArgFlags::HAS_DISTANCE;
				continue;
			}
			if (len == 3)
			{
				if (memcmp(keyStr, "lon", 4) == 0)
				{
					double lon = PyCoordinate::lonValue(value);
					if (lon == -1.0 && PyErr_Occurred()) return NULL;
					point.x = Mercator::xFromLon(lon);
					argFlags |= ArgFlags::HAS_X;
					continue;
				}
				else if (memcmp(keyStr, "lat", 4) == 0)
				{
					double lat = PyCoordinate::latValue(value);
					if (lat == -1.0 && PyErr_Occurred()) return NULL;
					point.y = Mercator::yFromLat(lat);
					argFlags |= ArgFlags::HAS_Y;
					continue;
				}
			}
			else if (memcmp(keyStr, "x", 2) == 0)
			{
				double x = PyFloat_AsDouble(value);
				if (x == -1.0 && PyErr_Occurred()) return NULL;
				point.x = static_cast<int32_t>(std::round(x));
				argFlags |= ArgFlags::HAS_X;
				continue;
			}
			else if (memcmp(keyStr, "y", 2) == 0)
			{
				double y = PyFloat_AsDouble(value);
				if (y == -1.0 && PyErr_Occurred()) return NULL;
				point.y = static_cast<int32_t>(std::round(y));
				argFlags |= ArgFlags::HAS_Y;
				continue;
			}
		}
	}

	if (argFlags != (ArgFlags::HAS_X | ArgFlags::HAS_Y | ArgFlags::HAS_DISTANCE))
	{
		PyErr_SetString(PyExc_TypeError, "Expected geometric object and distance");
		return NULL;
	}

	return self->withFilter(new PointDistanceFilter(distance, point));
}