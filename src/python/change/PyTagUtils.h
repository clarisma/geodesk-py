// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <Python.h>
#include <geodesk/feature/TagTablePtr.h>

namespace clarisma {
class Buffer;
}

using namespace geodesk;

class PyTagUtils
{
public:
	static bool isTagValue(PyObject* obj);
	static bool isAtomicTagValue(PyObject* obj);
	static int loadTags(PyObject* dict, TagTablePtr pTags, StringTable& strings);
	static PyObject* createTags(TagTablePtr pTags, StringTable& strings);
	static int setTag(PyObject* tags, PyObject* key, PyObject* value);
	static int merge(PyObject* destTags, PyObject* srcTags);
	static int equals(PyObject* dict, TagTablePtr tags, StringTable& strings);
	static void writeValue(clarisma::Buffer& out, PyObject* value);

private:
	static void writeAtomicValue(clarisma::Buffer& out,
		PyObject* value, bool insideTuple);
};
