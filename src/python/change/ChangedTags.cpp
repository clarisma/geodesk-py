// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "ChangedTags.h"
#include <clarisma/util/StringBuilder.h>
#include <geodesk/feature/FeatureStore.h>
#include <geodesk/feature/TagIterator.h>
#include "python/feature/PyFeature.h"
#include "python/util/util.h"


bool ChangedTags::addTag(PyObject* key, PyObject* value)
{
    assert(key);
    assert(value);
    if (!checkKey(key)) return false;
    PyObject* coerced = coerceValue(value);
    if (!coerced) return false;
    Py_INCREF(key);     // coerced is already a new ref
    tags_.emplace_back(key, coerced);
    return true;
}

bool ChangedTags::addFromSequence(PyObject* seq)
{
    Py_ssize_t n = PySequence_Fast_GET_SIZE(seq);
    PyObject **items = PySequence_Fast_ITEMS(seq);
    for (int i = 0; i < n; i++)
    {
        PyObject* item = items[i];
        PyObject* key;
        PyObject* value;
        if (PyUnicode_Check(item))		// coord pair or tag
        {
            if (i+1 >= n)
            {
                errorExpectedTag();
                return false;
            }
            key = item;
            i++;
            value = items[i];
        }
        else
        {
            PyObject *childSeq = PySequence_Fast(item,
                "Expected a key/value pair");
            if (!childSeq) return false;
            Py_ssize_t tupleSize = PySequence_Fast_GET_SIZE(childSeq);
            if (tupleSize != 2)
            {
                Py_DECREF(childSeq);
                errorExpectedTag();
                return false;
            }
            key = PyTuple_GET_ITEM(childSeq, 0);
            value = PyTuple_GET_ITEM(childSeq, 1);
            Py_DECREF(childSeq);
            // TODO: is this safe, or do we need to keep the ref???
        }
        if (!addTag(key, value)) return false;
    }
    return true;

}


void ChangedTags::errorExpectedTag()
{
    PyErr_SetString(PyExc_TypeError, "Expected a key/value pair");
}


PyObject* ChangedTags::coerceValue(PyObject* value)
{
    if (PyUnicode_Check(value)) return Python::newRef(value);
    return coerceNonStringValue(value, true);
}


PyObject* ChangedTags::coerceNonStringValue(PyObject* value, bool allowSequence)
{
    // check for bool must come before numeric
    if (value == Py_True || value == Py_False)
    {
        return PyUnicode_FromString(
            value == Py_True ? "yes" : "no");
            // TODO: cache yes/no?
    }
    if (PyNumber_Check(value)) return PyObject_Str(value);
    if (!allowSequence || !PySequence_Check(value))
    {
        PyErr_SetString(PyExc_ValueError,
            "tag value must be str, number, bool, or sequence thereof");
        return nullptr;
    }
    PyObject* seq = PySequence_Fast(value, "expected sequence");
    if (!seq) return nullptr;
    clarisma::StringBuilder str;

    Py_ssize_t n = PySequence_Fast_GET_SIZE(seq);
    PyObject** items = PySequence_Fast_ITEMS(seq);
    for (Py_ssize_t i = 0; i < n; ++i)
    {
        if (i > 0) str << ';';
        PyObject* item = items[i];
        if (PyUnicode_Check(item))
        {
            str << Python::stringAsStringView(item);
        }
        else
        {
            PyObject* coerced = coerceNonStringValue(item, false);
            if (!coerced)
            {
                Py_DECREF(seq);
                return nullptr;
            }
            str << Python::stringAsStringView(coerced);
            Py_DECREF(coerced);
        }
    }
    Py_DECREF(seq);
    return Python::toStringObject(static_cast<std::string_view>(str));
}


PyObject* ChangedTags::createTags(PyObject* original)
{
    if (original && Py_TYPE(original) == &PyFeature::TYPE)
    {
        PyFeature* feature = (PyFeature*)original;
        return createTags(feature->store, feature->feature);
    }
    return PyDict_New();
}


bool ChangedTags::hasTags(PyObject* original)
{
    if (!original || Py_TYPE(original) != &PyFeature::TYPE)
    {
        return false;
    }
    FeaturePtr feature = ((PyFeature*)original)->feature;
    assert(!feature.isNull());
    return !feature.tags().isEmpty() && !feature.isExceptionNode();
        // We need to check for exception node because we
        // don't treat geodesk::duplicate and geodesk::orphan
        // as real tags
}

PyObject* ChangedTags::createTags(FeatureStore* store, FeaturePtr feature)
{
    PyObject* dict = PyDict_New();
    if (!dict) return nullptr;

    if (feature.isExceptionNode())  [[unlikely]]
    {
        return dict;    // don't return geodesk:duplicate/orphan
    }

    // TODO: Use TalkWalker, can get global keys as shared string objects
    TagIterator iter(feature.tags(), store->strings());
    for (;;)
    {
        auto [keyStr, tagBits] = iter.next();
        if (keyStr == nullptr) break;

        PyObject* key = Python::toStringObject(keyStr->data(), keyStr->length());
        if (!key)
        {
            Py_DECREF(dict);
            return nullptr;
        }

        PyObject* value = iter.tags().valueAsString(tagBits, store->strings());
        if (!value)
        {
            Py_DECREF(key);
            Py_DECREF(dict);
            return nullptr;
        }

        if (PyDict_SetItem(dict, key, value) < 0)
        {
            Py_DECREF(key);
            Py_DECREF(value);
            Py_DECREF(dict);
            return nullptr;
        }

        Py_DECREF(key);
        Py_DECREF(value);
    }
    return dict;
}

bool ChangedTags::checkKey(PyObject* key)
{
    if (!PyUnicode_Check(key))
    {
        PyErr_SetString(PyExc_ValueError, "key must be str");
        return false;
    }
    if (PyUnicode_GetLength(key) == 0)
    {
        PyErr_SetString(PyExc_ValueError, "key string cannot be empty");
        return false;
    }
    return true;
}

bool ChangedTags::setKeyValue(PyObject* parent, PyObject* tags,
    PyObject* key, PyObject* value)
{
    (void)parent;  // currently unused; kept for future context needs

    if (!checkKey(key)) return false;

    if (value)
    {
        if (value != Py_None && (!PySequence_Check(value)
            || PySequence_Size(value) > 0))
        {
            PyObject* coerced = coerceValue(value);
            if (!coerced) return false;
            int rc = PyDict_SetItem(tags, key, coerced);
            Py_DECREF(coerced);
            return rc == 0;
        }
    }
    if (PyDict_DelItem(tags, key) == 0) return true;
    // Treat "missing key" as a no-op, not an error
    if (!PyErr_ExceptionMatches(PyExc_KeyError)) return false;
    PyErr_Clear();
    return true;
}







