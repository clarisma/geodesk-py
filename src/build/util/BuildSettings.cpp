// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "BuildSettings.h"
#include "feature/ZoomLevels.h"
#include "BuildSettings_lookup.cxx"

#ifdef GEODESK_PYTHON
#include "python/util/util.h"

const BuildSettings::SetterMethod BuildSettings::SETTER_METHODS[] =
{
    &setAreaTags,
    &setExcludedKeys,
    &setIdIndexing,
    &setIndexedKeys,
    &setKeyIndexMinFeatures,
    &setMaxKeyIndexes,
    &setMaxStrings,
    &setMaxTiles,
    &setMinStringUsage,
    &setMinTileDensity,
    &setProperties,
    &setRTreeBranchSize,
    &setSource,
    &setThreads,
    &setUpdatable,
    &setZoomLevels,
};

int BuildSettings::setAreaTags(PyObject* arg)
{
    return 0;
}

int BuildSettings::setExcludedKeys(PyObject* arg)
{
    return 0;
}

int BuildSettings::setIdIndexing(PyObject* arg)
{
    return 0;
}

int BuildSettings::setKeyIndexMinFeatures(PyObject* arg)
{
    return 0;
}

int BuildSettings::setMaxKeyIndexes(PyObject* arg)
{
    return 0;
}

int BuildSettings::setMinStringUsage(PyObject* arg)
{
    int64_t v = PyLong_AsLongLong(arg);
    if (v < 0 && PyErr_Occurred()) return -1;
    setMinStringUsage(v);
    return 0;
}

int BuildSettings::setMinTileDensity(PyObject* arg)
{
    int64_t v = PyLong_AsLongLong(arg);
    if (v < 0 && PyErr_Occurred()) return -1;
    setMinTileDensity(v);
    return 0;
}

int BuildSettings::setProperties(PyObject* arg)
{
    return 0;
}

int BuildSettings::setRTreeBranchSize(PyObject* arg)
{
    return 0;
}

int BuildSettings::setSource(PyObject* arg)
{
    std::string_view s = Python::getStringView(arg);
    if (!s.data()) return -1;
    setSource(s);
    return 0;
}

int BuildSettings::setThreads(PyObject* arg)
{
    int64_t v = PyLong_AsLongLong(arg);
    if (v < 0 && PyErr_Occurred()) return -1;
    setThreadCount(v);
    return 0;
}

int BuildSettings::setUpdatable(PyObject* arg)
{
    return 0;
}


int BuildSettings::setZoomLevels(PyObject* arg)
{
	uint32_t levels = 1;
    if (!PySequence_Check(arg))
    {
        PyErr_SetString(PyExc_TypeError, "Must be a sequence of numbers");
        return -1;
    }
    Py_ssize_t len = PySequence_Length(arg);
    for (Py_ssize_t i = 0; i < len; i++) 
    {
        PyObject* item = PySequence_GetItem(arg, i);
        if (!item) return -1;
        if (!PyNumber_Check(item))
        {
            PyErr_SetString(PyExc_TypeError, "Must be a sequence of numbers");
            Py_DECREF(item);
            return -1;
        }
        int zoom = PyLong_AsLong(item);
        Py_DECREF(item);
        if (zoom < 0 || zoom > 12)
        {
            PyErr_SetString(PyExc_ValueError, "Zoom level must be from 0 to 12");
            return -1;
        }
        levels |= 1 << zoom;
    }

    zoomLevels_ = ZoomLevels(levels);
    zoomLevels_.check();
    return 0;
}


int BuildSettings::setIndexedKeys(PyObject* arg)
{
    PyObject* list = PySequence_Fast(arg, "Must be a sequence of strings");
    if (!list) return -1;
    Py_ssize_t len = PySequence_Fast_GET_SIZE(list);
    for (Py_ssize_t i = 0; i < len; i++)
    {
        int category = i + 1;
        PyObject* item = PySequence_Fast_GET_ITEM(list, i);
        PyTypeObject* type = Py_TYPE(item);
        if (type == &PyUnicode_Type)
        {
            if (addIndexedKey(item, category) < 0)
            {
                Py_DECREF(list);
                return -1;
            }
        }
        else
        {
            PyObject* tuple = PySequence_Fast(item, "Items must be strings or tuples of strings");
            Py_ssize_t tupleLen = PySequence_Fast_GET_SIZE(tuple);
            for (Py_ssize_t i2 = 0; i2 < len; i2++)
            {
                PyObject* tupleItem = PySequence_Fast_GET_ITEM(tuple, i2);
                if (addIndexedKey(item, category) < 0)
                {
                    Py_DECREF(tuple);
                    Py_DECREF(list);
                    return -1;
                }
            }
        }
    }
    Py_DECREF(list);
}

int BuildSettings::addIndexedKey(PyObject* obj, int category)
{
    std::string_view key = Python::getStringView(obj);
    if (!key.data()) return -1;
    addIndexedKey(key, category);
    return 0;
}


int BuildSettings::setMaxStrings(PyObject* arg)
{
    int64_t v = PyLong_AsLongLong(arg);
    if (v < 0 && PyErr_Occurred()) return -1;
    setMaxStrings(v);
    return 0;
}

int BuildSettings::setMaxTiles(PyObject* arg)
{
    int64_t v = PyLong_AsLongLong(arg);
    if (v < 0 && PyErr_Occurred()) return -1;
    setMaxTiles(v);
    return 0;
}


int BuildSettings::setOptions(PyObject* dict)
{
    PyObject* key;
    PyObject* value;
    Py_ssize_t pos = 0;
    while (PyDict_Next(dict, &pos, &key, &value))
    {
        // References are borrowed, don't dec refcount

        Py_ssize_t len;
        const char* keyStr = PyUnicode_AsUTF8AndSize(key, &len);
        if (!keyStr) return -1;
        Attribute* attr = BuildSettings_AttrHash::lookup(keyStr, len);
        if (!attr)
        {
            PyErr_Format(PyExc_TypeError, "Invalid option: %s", keyStr);
            return -1;
        }
        try
        {
            if ((this->*SETTER_METHODS[attr->index])(value) < 0)
            {
                PyObject* msg = Python::getCurrentExceptionMessage();
                PyErr_Format(PyExc_ValueError, "%s: %s", keyStr, PyUnicode_AsUTF8(msg));
                Py_XDECREF(msg);
                return -1;
            }
        }
        catch (ValueException& ex)
        {
            PyErr_Format(PyExc_ValueError, "%s: %s", keyStr, ex.what());
            return -1;
        }
    }
    return 0;
}

#endif

void BuildSettings::addIndexedKey(std::string_view key, int category)
{
    // TODO: split string at / to get multiple keys
    indexedKeyStrings_.push_back(key);
    indexedKeyCategories_.push_back(static_cast<uint8_t>(category));
}

void BuildSettings::setSource(const std::string_view path)
{
    sourcePath_ = path;
    // TODO: Check if file exists, adjust extension
}
