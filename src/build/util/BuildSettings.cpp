// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "BuildSettings.h"
#include <cstring>
#include <common/cli/Console.h>
#include "feature/ZoomLevels.h"
#include "BuildSettings_lookup.cxx"

#ifdef GEODESK_PYTHON
#include "python/util/util.h"

const BuildSettings::SetterMethod BuildSettings::SETTER_METHODS[] =
{
    &BuildSettings::setAreaTags,
    &BuildSettings::setExcludedKeys,
    &BuildSettings::setIdIndexing,
    &BuildSettings::setIndexedKeys,
    &BuildSettings::setKeyIndexMinFeatures,
    &BuildSettings::setMaxKeyIndexes,
    &BuildSettings::setMaxStrings,
    &BuildSettings::setMaxTiles,
    &BuildSettings::setMinStringUsage,
    &BuildSettings::setMinTileDensity,
    &BuildSettings::setProperties,
    &BuildSettings::setRTreeBranchSize,
    &BuildSettings::setSource,
    &BuildSettings::setThreads,
    &BuildSettings::setUpdatable,
    &BuildSettings::setZoomLevels,
};

int BuildSettings::setAreaTags(PyObject* arg)
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


int BuildSettings::setIndexedKeys(PyObject* list)
{
    if (!PyList_Check(list))
    {
        PyErr_SetString(PyExc_TypeError, "Must be a sequence of strings");
        return -1;
    }
    Py_ssize_t len = PyList_GET_SIZE(list);
    //Console::msg("There are %d indexed key entries", len);
    printf("There are %zd indexed key entries\n", len);
    for (Py_ssize_t i = 0; i < len; i++)
    {
        //Console::msg("Indexed key entry %d", i);
        printf("Indexed key entry %zd\n", i);
        // int category = i + 1;
        /*
        PyObject* item = PyList_GET_ITEM(list, i);
        if(item == NULL)
        {
            printf("NULL!!!!");
            //Console::msg("NULL item at index %d!", i);
            continue;
        }
         */
        /*
        PyTypeObject* type = Py_TYPE(item);
        if (type == &PyUnicode_Type)
        {
            if (addIndexedKey(item, category) < 0) return -1;
        }
        else
        {
            PyObject* tuple = PySequence_Fast(item, "Items must be strings or tuples of strings");
            Py_ssize_t tupleLen = PySequence_Fast_GET_SIZE(tuple);
            for (Py_ssize_t i2 = 0; i2 < tupleLen; i2++)
            {
                PyObject* tupleItem = PySequence_Fast_GET_ITEM(tuple, i2);
                if (addIndexedKey(tupleItem, category) < 0)
                {
                    Py_DECREF(tuple);
                    return -1;
                }
            }
        }
         */
    }
}


int BuildSettings::setExcludedKeys(PyObject* arg)
{
    // TODO
    return 0; 
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
