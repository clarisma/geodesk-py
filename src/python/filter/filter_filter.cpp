// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "filters.h"
#include "python/feature/PyFeature.h"
#include "python/util/util.h"

class PythonFilter : public Filter
{
public:
	PythonFilter(PyObject* function) :
		function_(function)
	{
		Py_INCREF(function);
	}

	~PythonFilter() override
	{
		Py_DECREF(function_);
	}

	bool accept(FeatureStore* store, FeatureRef feature, FastFilterHint fast) const override
	{
		// The thread needs to acquire the GIL
		PyGILState_STATE gstate;
		gstate = PyGILState_Ensure();

		PyFeature* featureObj = PyFeature::create(store, feature, Py_None);
		PyObject* result = PyObject_CallOneArg(function_, featureObj);
		bool res;
		if (result == NULL)
		{
			// TODO: pass exception to query
			PyErr_Clear();
			res = false;
		}
		else
		{
			res = PyObject_IsTrue(result) == 1;
			// TODO: PyObject_IsTrue can technically fail with -1, but how?
			Py_DECREF(result);
		}
		Py_DECREF(feature);
		
		// Release the GIL
		PyGILState_Release(gstate);
		return res;
	}
private:
	PyObject* function_;
};

PyFeatures* filters::pythonFilter(PyFeatures* self, PyObject* args, PyObject* kwargs)
{
	PyObject* function = Python::checkSingleArg(args, kwargs, "function");
	if (function == NULL) return NULL;

	// TODO: handle bad_alloc
	return self->withFilter(new PythonFilter(function));
}