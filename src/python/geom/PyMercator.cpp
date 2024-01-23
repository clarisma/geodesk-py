// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyMercator.h"
#include <geos_c.h>
#include "python/Environment.h"
#include "PyCoordinate.h"
#include "geom/LengthUnit.h"
#include "geom/Mercator.h"
#include "geom/geos/MercatorCoordinateFilter.h"
#include "python/util/util.h"
#include <string.h>

// For now, we return a copies of transformed shapes, to conform
// to what the Shapely library does
// #define GEODESK_MERCATOR_TRANSFORM_INPLACE 1

// TODO: What should happen to Coordinate objects if passed to to_mercator?

/**
 * Forms:
 *   to_mercator(<Coordinate>) -> Coordinate (kind of pointless)
 *   to_mercator(<Geometry>)   -> Geometry
 *   to_mercator((<lon>, <lat>)) -> Coordinate
 *   to_mercator(<lon>, <lat>)   -> Coordinate
 *   to_mercator(lat=, lon=) -> Coordinate
 *   to_mercator(latlon=<coords>) -> Sequence of Coordinates
 *   to_mercator(lonlat=<coords>) -> Sequence of Coordinates
 *   to_mercator(<unit>=<distance>, lat=|y=) -> float
 */
PyObject* PyMercator::to_mercator(PyObject* self, PyObject* args, PyObject* kwargs)
{
	Py_ssize_t argCount = PySequence_Length(args);
	if (kwargs)
	{
		if (argCount != 0)
		{
			PyErr_SetString(PyExc_TypeError, "Invalid combination of arguments");
			return NULL;
		}
		enum Keywords
		{
			LON = 1,
			LAT = 2,
			LONLAT = 4,
			LATLON = 8,
			Y = 16,
			UNITS = 32
		};

		int keywords = 0;
		int units;
		double distance;
		double lon;
		double lat;
		double y;
		PyObject* coords;

		PyObject* key;
		PyObject* value;
		Py_ssize_t pos = 0;
		while (PyDict_Next(kwargs, &pos, &key, &value))
		{
			// References are borrowed, don't dec refcount
			Py_ssize_t len;
			const char* keyStr = PyUnicode_AsUTF8AndSize(key, &len);
			if (!keyStr) return NULL;
			int keyword = 0;
			if (len == 3)
			{
				if (memcmp(keyStr, "lon", 4) == 0)
				{
					keyword = LON;
					lon = PyCoordinate::lonValue(value);
					if (lon == -1.0 && PyErr_Occurred()) return NULL;
				}
				else if (memcmp(keyStr, "lat", 4) == 0)
				{
					keyword = LAT;
					lat = PyCoordinate::latValue(value);
					if (lat == -1.0 && PyErr_Occurred()) return NULL;
				}
			}
			else if (len == 6)
			{
				if (memcmp(keyStr, "lonlat", 7) == 0)
				{
					keyword = LONLAT;
					coords = value;
				}
				else if (memcmp(keyStr, "latlon", 7) == 0)
				{
					keyword = LATLON;
					coords = value;
				}
			}
			else if (memcmp(keyStr, "y", 2) == 0)
			{
				keyword = Y;
				y = PyFloat_AsDouble(value);
				if (y == -1.0 && PyErr_Occurred()) return NULL;
			}
			if (keyword == 0)
			{
				units = LengthUnit::unitFromString(std::string_view(keyStr, len));
				if (units >= 0)
				{
					keyword = UNITS;
					distance = PyFloat_AsDouble(value);
					if (distance == -1.0 && PyErr_Occurred()) return NULL;
				}
				else
				{
					return Python::badKeyword(keyStr);
				}
			}
			keywords |= keyword;
		}
		if (keywords == (LON | LAT))
		{
			return PyCoordinate::create(Mercator::xFromLon(lon), Mercator::yFromLat(lat));
		}
		if (keywords == LONLAT)
		{
			return coordinatesToMercator(coords, CoordinateOrder::ORDER_LONLAT);
		}
		if (keywords == LATLON)
		{
			return coordinatesToMercator(coords, CoordinateOrder::ORDER_LATLON);
		}
		if (keywords == (UNITS | Y))
		{
			return PyFloat_FromDouble(Mercator::unitsFromMeters(
				LengthUnit::toMeters(distance, units), y));
		}
		if (keywords == (UNITS | LAT))
		{
			return PyFloat_FromDouble(Mercator::unitsFromMeters(
				LengthUnit::toMeters(distance, units), Mercator::yFromLat(lat)));
		}
		if (keywords == LON)
		{
			return PyFloat_FromDouble(Mercator::xFromLon(lon));
		}
		if (keywords == LAT)
		{
			return PyFloat_FromDouble(Mercator::yFromLat(lat));
		}
		if (keywords == UNITS)
		{
			PyErr_SetString(PyExc_TypeError, "Requires 'lat' or 'y' because scale depends on latitude");
			return NULL;
		}

		PyErr_SetString(PyExc_TypeError, "Invalid combination of arguments");
		return NULL;
	}

	if (argCount == 0)
	{
		PyErr_SetString(PyExc_TypeError, "Missing argument");
		return NULL;
	}
	if (argCount == 1)
	{
		PyObject* arg = PyTuple_GET_ITEM(args, 0);
		geos::geom::Geometry* geom;
		if (Environment::get().getGeosGeometry(arg, (GEOSGeometry**)&geom))
		{
			ToMercatorCoordinateFilter filter;
			#ifdef GEODESK_MERCATOR_TRANSFORM_INPLACE
			geom->apply_rw(&filter);
			return Python::newRef(arg);
			#else			
			geos::geom::Geometry* copy = geom->clone().release();
			// TODO: error check (e.g. out of memory)
			copy->apply_rw(&filter);
			return Environment::get().buildShapelyGeometry((GEOSGeometry*)copy);
			#endif			
			// Creating a copy first, unsurprisingly, more than doubles runtime
		}
		else
		{
			PyErr_Clear(); // TODO
		}
		if (PySequence_Check(arg))
		{
			return coordinatesToMercator(arg, CoordinateOrder::ORDER_LONLAT);
		}
		PyTypeObject* type = Py_TYPE(arg);
		return Python::badArgumentType(type);
	}
	if (argCount == 2)
	{
		PyObject* firstArg = PyTuple_GET_ITEM(args, 0);
		if (Python::isNumeric(firstArg))
		{
			double lon = PyCoordinate::lonValue(firstArg);
			if(lon == -1.0 && PyErr_Occurred()) return NULL;
			double lat = PyCoordinate::latValue(PyTuple_GET_ITEM(args, 1));
			if (lat == -1.0 && PyErr_Occurred()) return NULL;
			return coordinateFromLonLat(lon, lat);
		}
		// fall through
	}
	return coordinatesToMercator(args, CoordinateOrder::ORDER_LONLAT);
}


PyCoordinate* PyMercator::coordinateFromLonLat(double lon, double lat)
{
	return PyCoordinate::create(Mercator::xFromLon(lon), Mercator::yFromLat(lat));
}

PyCoordinate* PyMercator::coordinateFromLonLat(CoordinateOrder order, PyObject* first, PyObject* second)
{
	double lon = PyCoordinate::lonValue(
		order == CoordinateOrder::ORDER_LONLAT ? first : second);
	if (lon == -1.0 && PyErr_Occurred()) return NULL;
	double lat = PyCoordinate::latValue(
		order == CoordinateOrder::ORDER_LONLAT ? second : first);
	if (lat == -1.0 && PyErr_Occurred()) return NULL;
	return coordinateFromLonLat(lon, lat);
}

PyCoordinate* PyMercator::coordinateFromPair(CoordinateOrder order, PyObject* obj)
{
	// Leave PyCoordinate unchanged, as it is already in Mercator projection
	if (Py_TYPE(obj) == &PyCoordinate::TYPE) return Python::newRef((PyCoordinate*)obj);

	PyObject* item = PySequence_Fast(obj, "Expected coordinate pair");
	if (item == NULL) return NULL;
	Py_ssize_t itemLen = PySequence_Fast_GET_SIZE(item);
	if (itemLen != 2)
	{
		PyErr_SetString(PyExc_TypeError, "Expected coordinate pair");
		Py_DECREF(item);
		return NULL;
	}
	PyObject** pair = PySequence_Fast_ITEMS(item);
	PyCoordinate* coord = coordinateFromLonLat(order, pair[0], pair[1]);
	Py_DECREF(item);
	return coord;
}

PyObject* PyMercator::coordinatesToMercator(PyObject* seq, CoordinateOrder order)
{
	// Leave PyCoordinate unchanged, as it is already in Mercator projection
	if (Py_TYPE(seq) == &PyCoordinate::TYPE) return Python::newRef((PyCoordinate*)seq);

	seq = PySequence_Fast(seq, "Expected sequence or iterable");
	if (seq == NULL) return NULL;
	Py_ssize_t len = PySequence_Fast_GET_SIZE(seq);
	PyObject* res;
	if (len == 0)
	{
		res = PyList_New(0);
	}
	else
	{
		PyObject** p = PySequence_Fast_ITEMS(seq);
		if (Python::isNumeric(*p))
		{
			// Flat sequence of coordinate pairs

			if (len % 1)
			{
				PyErr_SetString(PyExc_TypeError, "Expected a sequence of coordinate pairs");
				Py_DECREF(seq);
				return NULL;
			}
			len /= 2;
			res = PyList_New(len);
			if (res)
			{
				for (int i = 0; i < len; i++)
				{
					PyObject* lonObj = *p++;
					PyObject* latObj = *p++;
					PyCoordinate* coord = coordinateFromLonLat(order, lonObj, latObj);
					if (coord == NULL)
					{
						Py_DECREF(res);
						Py_DECREF(seq);
						return NULL;
					}
					PyList_SET_ITEM(res, i, coord);
				}
			}
		}
		else
		{
			// sequence of coordinate tuples

			res = PyList_New(len);
			if (res)
			{
				for (int i = 0; i < len; i++)
				{
					PyCoordinate* coord = coordinateFromPair(order, *p++);
					if (coord == NULL)
					{
						Py_DECREF(res);
						Py_DECREF(seq);
						return NULL;
					}
					PyList_SET_ITEM(res, i, coord);
				}
			}
		}
	}
	Py_DECREF(seq);
	return res;
}


/**
 * Forms:
 *   from_mercator(<Coordinate>) -> Coordinate (kind of pointless)
 *   from_mercator(<Geometry>)   -> Geometry
 *   from_mercator((<x>, <y>)) -> Coordinate
 *   from_mercator(<coord-sequence>)   -> Coordinate/Sequence of Coordinate
 *   to_mercator(<units>, units=<units>, lat=|y=) -> float
 */

// TODO: Should this be able to take a Feature? This avoids need to create
//  a second copy vs. calling from_mercator(f.shape)

PyObject* PyMercator::from_mercator(PyObject* self, PyObject* args, PyObject* kwargs)
{
	static const char* KEYWORDS[] = { "geom", "unit", "lat", "y", NULL };
	static constexpr double DOUBLE_MIN = std::numeric_limits<double>::lowest();
	PyObject* arg;
	const char* units;
	double lat = DOUBLE_MIN;
	long long y = LLONG_MIN;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|sdL", const_cast<char**>(KEYWORDS),
		&arg, &units, &lat, &y)) 
	{
		return NULL;  // If parsing fails, return NULL
	}

	if (PyNumber_Check(arg))
	{
		// Scalar value (length)
		double length = PyFloat_AsDouble(arg);
		int unit = LengthUnit::unitFromString(std::string_view(units));
		if (unit < 0)
		{
			PyErr_Format(PyExc_ValueError, "Invalid units: %s (Must be %s)", 
				units, LengthUnit::VALID_UNITS);
			return NULL;
		}
		// TODO: Right now, lat takes precendence over y if both
		//   are specified; raise exception instead?
		if (lat > DOUBLE_MIN)
		{
			// TODO: lat range check & clamp
			y = Mercator::yFromLat(lat);
		}
		else if (y == LLONG_MIN)
		{
			PyErr_SetString(PyExc_TypeError, "Requires 'lat' or 'y' because scale depends on latitude");
			return NULL;
		}
		return PyFloat_FromDouble(
			LengthUnit::fromMeters(Mercator::metersPerUnitAtY(y) * length, unit));
	}
	else
	{
		// units and lat/y are ignored -- TODO: Should it be an error
		// if user supplies them?

		geos::geom::Geometry* geom;
		if (Environment::get().getGeosGeometry(arg, (GEOSGeometry**)&geom))
		{
			FromMercatorCoordinateFilter filter;
			#ifdef GEODESK_MERCATOR_TRANSFORM_INPLACE
			geom->apply_rw(&filter);
			return Python::newRef(arg);
			#else			
			geos::geom::Geometry* copy = geom->clone().release();
				// TODO: error check (e.g. out of memory)
			copy->apply_rw(&filter);
			return Environment::get().buildShapelyGeometry((GEOSGeometry*)copy);
			#endif		
			//  Creating a copy first, unsurprisingly, more than doubles runtime
		}
		else
		{
			PyErr_Clear(); // TODO
			return Python::badArgumentType(Py_TYPE(arg));
		}
	}
}

