// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "module.h"
#include <iostream>
#include "python/Environment.h"
#include "python/feature/PyFeature.h"
#include "python/feature/PyTags.h"
#include "python/format/PyFormatter.h"
#include "python/format/PyMap.h"
#include "python/geom/PyBox.h"
#include "python/geom/PyCoordinate.h"
#include "python/geom/PyMercator.h"
// #include "python/geom/PyRTree.h"
#include "python/query/PyFeatures.h"
#include "python/query/PyQuery.h"
#include "python/query/PyTile.h"
#include "python/util/PyBinder.h"
#include "python/util/PyFastMethod.h"
#include <clarisma/util/log.h>

static PyMethodDef GEODESK_METHODS[] = 
{
    { "lonlat", (PyCFunction)PyCoordinate::createLonLat, METH_VARARGS,
      "Creates a Coordinate from (lon, lat)"},
    { "latlon", (PyCFunction)PyCoordinate::createLatLon, METH_VARARGS,
      "Creates a Coordinate from (lat, lon)"},
    { "to_mercator", (PyCFunction)PyMercator::to_mercator, METH_VARARGS | METH_KEYWORDS,
     "Converts coordinates or measurements from WGS-84 to Mercator projection"},
    { "from_mercator", (PyCFunction)PyMercator::from_mercator, METH_VARARGS | METH_KEYWORDS,
     "Converts coordinates or measurements from Mercator projection to WGS-84"},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

static PyModuleDef GEODESK_MODULE =
{
    PyModuleDef_HEAD_INIT,
    "geodesk",
    "GeoDesk Query Engine",
    -1,                         // TODO: use this for size of module state
    GEODESK_METHODS,
    nullptr, nullptr, nullptr, nullptr
};

int createPrivateType(PyObject* module, PyTypeObject* type)
{
    if (PyType_Ready(type) < 0)
    {
        Py_DECREF(module);
        return -1;
    }
    return 0;
}

// TODO: Could use PyModule_AddType(PyObject *module, PyTypeObject *type) since 3.10
int createPublicType(PyObject* module, const char* name, PyTypeObject* type)
{
    if (PyType_Ready(type) < 0)
    {
        Py_DECREF(module);
        return -1;
    }
    if (PyModule_AddObject(module, name, (PyObject*)type) < 0) 
    {
        Py_DECREF(type);
        Py_DECREF(module);
        return -1;
    }
    return 0;
}

#ifdef GEODESK_TEST_PERFORMANCE
volatile uint32_t performance_blackhole;
#endif 

extern "C" PyMODINIT_FUNC PyInit_geodesk()
{
#ifdef GEODESK_TEST_PERFORMANCE
    printf("\n=== Test build, not for release ===\n");
#endif
    LOG("PyInit_geodesk...");
    

    if (Environment::get().init() < 0) return NULL;

    PyObject* module = PyModule_Create(&GEODESK_MODULE);
    if (!module) return nullptr;
    if(createPublicType(module, "Box", &PyBox::TYPE) < 0) return nullptr;
    if (createPublicType(module, "Coordinate", &PyCoordinate::TYPE) < 0) return nullptr;
    if (createPublicType(module, "Feature", &PyFeature::TYPE) < 0) return nullptr;
    if (createPublicType(module, "Features", &PyFeatures::TYPE) < 0) return nullptr;
    if (createPublicType(module, "Map", &PyMap::TYPE) < 0) return nullptr;
    // if (createPublicType(module, "RTree", &PyRTree::TYPE) < 0) return nullptr;

    if (createPrivateType(module, &PyQuery::TYPE) < 0) return nullptr;
    if (createPrivateType(module, &PyTags::TYPE) < 0) return nullptr;
    if (createPrivateType(module, &PyTagIterator::TYPE) < 0) return nullptr;
    if (createPrivateType(module, &PyMemberIterator::TYPE) < 0) return nullptr;
    if (createPrivateType(module, &PyWayNodeIterator::TYPE) < 0) return nullptr;
    if (createPrivateType(module, &PyParentRelationIterator::TYPE) < 0) return nullptr;
    if (createPrivateType(module, &PyNodeParentIterator::TYPE) < 0) return nullptr;
    if (createPrivateType(module, &PyAnonymousNode::TYPE) < 0) return nullptr;
    if (createPrivateType(module, &PyFastMethod::TYPE) < 0) return nullptr;
    if (createPrivateType(module, &PyBinder::TYPE) < 0) return nullptr;
    if (createPrivateType(module, &PyFormatter::TYPE) < 0) return nullptr;
    if (createPrivateType(module, &PyTile::TYPE) < 0) return nullptr;
    // if (createPrivateType(module, &PyRTreeQuery::TYPE) < 0) return nullptr;

    Python::createDirMethod(&PyFeatures::TYPE, (PyCFunctionWithKeywords)&PyFeatures::dir);

    PyObject* exc = PyErr_NewException("geodesk.QueryError", NULL, NULL);
    if (!exc)
    {
        Py_DECREF(module);
        return nullptr;
    }
    if (PyModule_AddObject(module, "QueryError", exc) < 0)
    {
        Py_DECREF(exc);
        Py_DECREF(module);
        return nullptr;
    }
    Environment::get().queryException_ = exc;

    /*
    PyObject* submodule = PyInit_geodesk_filter();
    if (!submodule)
    {
        Py_DECREF(module);
        return nullptr;
    }
    if (PyModule_AddObject(module, "filter", submodule) < 0)
    {
        Py_DECREF(submodule);
        Py_DECREF(module);
        return nullptr;
    }

    // TODO: experimental
    PyModule_AddObject(module, "__package__", PyUnicode_FromString("geodesk"));
    */

    LOG("Successfully created geodesk module.");

    return module;
}

