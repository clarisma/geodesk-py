// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "util.h"
#include <stdexcept>
#include "PyFastMethod.h"


PyObject* Python::createSet(const char** strings) 
{
    // Create a new empty Python set
    PyObject* set = PySet_New(NULL);
    if (!set) return NULL; // Failed to create a new set, likely due to memory allocation failure

    // Iterate through the provided C strings and add each to the Python set
    for (; *strings != NULL; ++strings) 
    {
        PyObject* str = PyUnicode_FromString(*strings);
        if (!str) 
        {
            Py_DECREF(set); // Clean up before returning
            return NULL; // Failed to create a new Python string
        }

        // Add the string to the set. 
        // TODO: Check if this operation "steals" the reference of str
        if (PySet_Add(set, str) == -1) 
        {
            Py_DECREF(str);  // If PySet_Add fails, we need to decrement the reference ourselves
            Py_DECREF(set); // Clean up before returning
            return NULL; // Failed to add to the set
        }
        Py_DECREF(str); // Decrement the reference count since the set holds a reference after adding
    }
    return set;
}


PyObject* Python::createList(const char** strings, size_t count)
{
    PyObject* list = PyList_New(count);
    if (!list) return NULL;

    for (size_t i = 0; i < count; i++) 
    {
        // Convert C-string to Python unicode string
        PyObject* str = PyUnicode_FromString(strings[i]);
        if (!str)
        {
            Py_DECREF(list);
            return NULL;
        }
        // Note: PyList_SetItem steals a reference, so we don't need to DECREF pyString.
        PyList_SetItem(list, i, str);
    }
    return list;
}



// TODO: works only if object implements the mapping protocol
PyObject* Python::formatString(PyObject* templateString, PyObject* object)
{
    // Get the format method of the string
    PyObject* format_method = PyObject_GetAttrString(templateString, "format_map");

    // Check if method retrieval was successful
    if (!format_method) return NULL;
    
    // Call the format method using the object
    PyObject* args = PyTuple_Pack(1, object);
    PyObject* result_string = PyObject_CallObject(format_method, args);

    // Cleanup
    Py_DECREF(format_method);
    Py_DECREF(args);
    return result_string; // This is the formatted string. Remember to DECREF it after using it.
}



std::string_view Python::stringAsStringView(PyObject* strObj) 
{
    Py_ssize_t size;
    const char* data = PyUnicode_AsUTF8AndSize(strObj, &size);
    if (!data) 
    {
        throw std::runtime_error("Failed to get UTF-8 data from string");
    }
    return std::string_view(data, size);
}

// Usage:
// PyObject* py_str = ...;
// std::string_view sv = PyString_AsStringView(py_str);


PyObject* Python::checkSingleArg(PyObject* args, PyObject* kwargs, const char* what)
{
    Py_ssize_t argCount = PySequence_Length(args);
    if (argCount != 1 || kwargs != NULL)
    {
        PyErr_Format(PyExc_TypeError, "Expected single argument (%s)", what);
        return NULL;
    }
    return PyTuple_GET_ITEM(args, 0);
}

PyObject* Python::checkType(PyObject* arg, PyTypeObject* type)
{
    if (Py_TYPE(arg) != type)
    {
        PyErr_Format(PyExc_TypeError, "Expected %s (instead of %s)", 
            type->tp_name, arg->ob_type->tp_name);
        return NULL;
    }
    return arg;
}

PyObject* Python::checkType(PyObject* arg, PyTypeObject* type, const char* what)
{
    if (Py_TYPE(arg) != type)
    {
        PyErr_Format(PyExc_TypeError, "Expected %s (instead of %s)",
            what, arg->ob_type->tp_name);
        return NULL;
    }
    return arg;
}

PyObject* Python::checkNumeric(PyObject* arg)
{
    if (!PyNumber_Check(arg))
    {
        PyErr_Format(PyExc_TypeError, "Expected number (instead of %s)", 
            arg->ob_type->tp_name);
        return NULL;
    }
    return arg;
}


PyObject* Python::checkSingleArg(PyObject* args, PyObject* kwargs, PyTypeObject* type)
{
    PyObject* obj = checkSingleArg(args, kwargs, type->tp_name);
    if (obj)
    {
        int res = PyObject_IsInstance(obj, (PyObject*)type);
        if (res < 1)
        {
            if (res == 0)
            {
                PyErr_Format(PyExc_TypeError, "Expected %s (instead of %s)", 
                    type->tp_name, obj->ob_type->tp_name);
            }
            obj = NULL;
        }
    }
    return obj;
}


void Python::createDirMethod(PyTypeObject* type, PyCFunctionWithKeywords dirFunc)
{
    PyObject* method = (PyObject*)PyFastMethod::create((PyObject*)type, dirFunc);
    if (method) 
    {
        // Note: PyDict_SetItem does NOT steal ref
        PyDict_SetItemString(type->tp_dict, "__dir__", method);
        Py_DECREF(method);
    }
}


PyObject* Python::getCurrentExceptionMessage()
{
    PyObject* exc_type, * exc_value, * exc_traceback;
    PyErr_Fetch(&exc_type, &exc_value, &exc_traceback);
    PyObject* exc_value_str = PyObject_Str(exc_value);
    Py_XDECREF(exc_type);
    Py_XDECREF(exc_value);
    Py_XDECREF(exc_traceback);
    return exc_value_str;
}
