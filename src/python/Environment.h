// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <geos_c.h>
#include <Python.h>
#include <geodesk/match/Matcher.h>
#include "python/util/util.h"

using namespace geodesk;
class PyFeatures;

typedef PyObject* (*ShapelyCreateGeometryFunc)(GEOSGeometry* ptr, GEOSContextHandle_t ctx);
typedef char (*ShapelyGetGEOSGeometryFunc)(PyObject* obj, GEOSGeometry** out);

class Environment
{
public:
	Environment();
	~Environment();
	int init();

	enum Strings
	{
		BLANK = 0,
		NODE = 1,		// keep NODE/WAY/RELATION/INVALID in order
		WAY = 2,
		RELATION = 3,
		INVALID = 4,
	};

	static const int STRING_CONSTANT_COUNT = 5;

	static Environment& get() { return ENV; }

	GEOSContextHandle_t getGeosContext()
	{
		if (!geosContext_)
		{
			geosContext_ = GEOS_init_r();
			// TODO: handle case if init fails
			if (!geosContext_)
			{
				// TODO: This may not be the proper way to handle this
				PyErr_SetString(PyExc_RuntimeError, "Failed to initialize GEOS");
			}
			GEOSContext_setErrorHandler_r(geosContext_, reportGeosError);
		}
		return geosContext_;
	}

	PyObject* getShapelyModule()
	{
		// TODO: Research the liveness guarantee for this pointer;
		// do we need to hold a reference to the Shapely module?

		if (!shapelyModule_) shapelyModule_ = PyImport_ImportModule("shapely");
		return shapelyModule_;
	}

	void** initShapelyFunctions()
	{
		shapelyApiFunctions_ = (void**)PyCapsule_Import("shapely.lib._C_API", 0);
		if (!shapelyApiFunctions_)
		{
			PyErr_SetString(PyExc_ImportError, "Failed to import shapely C API");
		}
		return shapelyApiFunctions_;
	}

	void** getShapelyFunctions()
	{
		if (!shapelyApiFunctions_) initShapelyFunctions();
		return shapelyApiFunctions_;
	}

	/*
	 * Creates a Shapely object based on the given GEOS geometry. `geom`
	 * may be NULL if a previous GEOS operation has failed, in which case
	 * a Python exception is raised.
	 *
	 * @param geom  the GEOS geometry
	 * @returns the Shapely object, or NULL (in which case a Python
	 *   exception has been raised)
	 */
	PyObject* buildShapelyGeometry(GEOSGeometry* geom);

	bool getGeosGeometry(PyObject* obj, GEOSGeometry** pGeom);
	
	static void reportGeosError(const char* format, ...)
	{
		// TODO
		va_list args;
		va_start(args, format);
		vfprintf(stderr, format, args);
		va_end(args);
	}

	PyObject* getString(int code)
	{
		assert(code >= 0 && code < STRING_CONSTANT_COUNT);
		return Python::newRef(stringConstants_[code]);
	}

	static void clearAndLogException();

	// PyFeatures* getEmptyFeatures();
	PyObject* raiseQueryException(const char* format, ...);

private:
	static Environment ENV;

	GEOSContextHandle_t geosContext_;
	PyObject* shapelyModule_;
	void** shapelyApiFunctions_;
	// PyFeatures* emptyFeatures_;
	PyObject* queryException_;		// not owned, don't refcount
	// MatcherHolder allMatcher_;
	// TODO: Decide where to keep this: here of in FeatureStore
	// (should not be tied to FeatureStore)
	PyObject* stringConstants_[STRING_CONSTANT_COUNT];
	static const char* STRING_CONSTANTS[STRING_CONSTANT_COUNT];

	friend PyObject* PyInit__geodesk();
};

// Don't format string because tuple unpacking relies on exception being raised
// A simple string is more efficient
inline PyObject* raiseIndexOutOfRange(size_t index)
{
	PyErr_SetString(PyExc_IndexError, "Index out of range");
	// PyErr_Format(PyExc_IndexError, "Index out of range (%d)", index);
	return NULL;
}
