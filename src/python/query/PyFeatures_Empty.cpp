// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyFeatures.h"

PyObject* PyFeatures::Empty::iterFeatures(PyFeatures* self)
{
    // The PyFeatures itself is an empty iterator
    return Python::newRef(self);
}

PyObject* PyFeatures::Empty::countFeatures(PyFeatures*)
{
    return PyLong_FromLong(0);
}

int PyFeatures::Empty::isEmpty(PyFeatures*)
{
    return 1;
}

SelectionType PyFeatures::Empty::SUBTYPE =
{
    iterFeatures,
    countFeatures,
    isEmpty,
    containsFeature,
    getTiles
};