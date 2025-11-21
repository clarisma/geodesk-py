// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <geodesk/feature/FeaturePtr.h>
#include <vector>
#include "python/util/PythonRef.h"

using namespace geodesk;

class ChangedTags
{
public:
    struct Tag
    {
        PyObject* key;
        PyObject* value;
    };

    ~ChangedTags()
    {
        for (Tag tag : tags_)
        {
            Py_DECREF(tag.key);
            Py_XDECREF(tag.value);
        }
    }

    bool addTag(PyObject* key, PyObject* value);
    bool addFromSequence(PyObject* seq);

    static PyObject* createTags(FeatureStore* store, FeaturePtr feature);
    static PyObject* createTags(PyObject* original);
    static PyObject* coerceValue(PyObject* value);
    static bool checkKey(PyObject* key);
    static bool setKeyValue(PyObject* parent, PyObject* tags,
        PyObject* key, PyObject* value);
    static bool hasTags(PyObject* original);

private:
    static PyObject* coerceNonStringValue(PyObject* value, bool allowSequence);
    static void errorExpectedTag();

    std::vector<Tag> tags_;
};