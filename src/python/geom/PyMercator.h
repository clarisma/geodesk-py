// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <Python.h>

class PyCoordinate;

class PyMercator
{
public:
	enum CoordinateOrder
	{
		ORDER_LONLAT = 1,
		ORDER_LATLON = -1
	};

	static PyObject* to_mercator(PyObject* self, PyObject* args, PyObject* kwargs);
	static PyObject* from_mercator(PyObject* self, PyObject* args, PyObject* kwargs);

	static PyCoordinate* coordinateFromLonLat(double lon, double lat);
	static PyCoordinate* coordinateFromLonLat(CoordinateOrder order, PyObject* first, PyObject* second);
	static PyCoordinate* coordinateFromPair(CoordinateOrder order, PyObject* pair);
	static PyObject* coordinatesToMercator(PyObject* arg, CoordinateOrder order);
};

