// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyFeatures.h"
#include "build/GolBuilder.h"

PyFeatures* PyFeatures::build(PyObject* args, PyObject* kwds)
{
	GolBuilder::build(args, kwds);
	return (PyFeatures*)Python::newRef(Py_None); // TODO
}
