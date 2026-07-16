// Copyright (c) 2026 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyMercator.h"
#include <geodesk/geom/Area.h>
#include <geodesk/geom/SpatialUnit.h>

#include "geodesk/geom/geos/Geos.h"
#include "python/Environment.h"
#include "python/feature/PyFeature.h"
#include "python/geom/PyBox.h"


PyObject* PyMercator::area (PyObject* self, PyObject* args, PyObject* kwargs)
{
    static const char *const KEYWORDS[] = { "", "units", nullptr};

    PyObject* obj;
    const char* unitsArg = nullptr;

    if (PyArg_ParseTupleAndKeywords(
        args, kwargs, "O|s:area",
        const_cast<char **>(KEYWORDS), &obj, &unitsArg) == 0)
    {
        return NULL;
    }
    int units = unitsFromArg(unitsArg, false);
    if (units < 0) return NULL;

    double area = 0;

    PyTypeObject* type = Py_TYPE(obj);
    if (type == &PyFeature::TYPE)
    {
        PyFeature* f = static_cast<PyFeature*>(obj);
        area = Area::ofFeature(f->store, f->feature);
    }
    else if (type == &PyAnonymousNode::TYPE)
    {
        // do nothing
    }
    else if (type == &PyBox::TYPE)
    {
        PyBox* b = static_cast<PyBox*>(obj);
        area = b->area();
    }
    else
    {
        Environment& env = Environment::get();
        GEOSGeometry* geom;
        if (!env.getGeosGeometry(obj, &geom))
        {
            PyErr_Format(PyExc_TypeError,
                "Expected Geometry, Feature or Box (instead of %s)",
                        type->tp_name);
            return NULL;
        }

        if (!GEOSArea_r(env.getGeosContext(), geom, &area))
        {
            return Python::geosError("area");
        }
        int32_t y = Geos::getEnvelope(env.getGeosContext(), geom).center().y;
        double scale = Mercator::metersPerUnitAtY(y);
        area *= scale * scale;
    }
    return PyFloat_FromDouble(SpatialUnit::fromSquareMeters(area, units));
}

