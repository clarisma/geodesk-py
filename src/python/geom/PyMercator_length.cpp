// Copyright (c) 2026 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyMercator.h"
#include <geodesk/geom/Length.h>
#include <geodesk/geom/SpatialUnit.h>

#include "geodesk/geom/geos/Geos.h"
#include "python/Environment.h"
#include "python/feature/PyFeature.h"


PyObject* PyMercator::length (PyObject* self, PyObject* args, PyObject* kwargs)
{
    static const char *const KEYWORDS[] = { "", "units", nullptr};

    PyObject* obj;
    const char* unitsArg = nullptr;

    if (PyArg_ParseTupleAndKeywords(
        args, kwargs, "O|s:length",
        const_cast<char **>(KEYWORDS), &obj, &unitsArg) == 0)
    {
        return NULL;
    }
    int units = unitsFromArg(unitsArg, false);
    if (units < 0) return NULL;

    double len = 0;

    PyTypeObject* type = Py_TYPE(obj);
    if (type == &PyFeature::TYPE)
    {
        PyFeature* f = static_cast<PyFeature*>(obj);
        len = Length::ofFeature(f->store, f->feature);
    }
    else if (type == &PyAnonymousNode::TYPE)
    {
        // do nothing
    }
    else
    {
        Environment& env = Environment::get();
        GEOSGeometry* geom;
        if (!env.getGeosGeometry(obj, &geom))
        {
            PyErr_Format(PyExc_TypeError,
                "Expected Geometry or Feature (instead of %s)",
                        type->tp_name);
            return NULL;
        }

        if (!GEOSLength_r(env.getGeosContext(), geom, &len))
        {
            return Python::geosError("length");
        }
        int32_t y = Geos::getEnvelope(env.getGeosContext(), geom).center().y;
        len *= Mercator::metersPerUnitAtY(y);
    }
    return PyFloat_FromDouble(SpatialUnit::fromMeters(len, units));
}

