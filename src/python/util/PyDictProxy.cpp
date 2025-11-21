// Copyright (c) 2025 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PyDictProxy.h"
#include "util.h"

PyDictProxy* PyDictProxy::create(PyObject* dict, PyObject* context,
    ChangeItemFunc change)
{
    assert (PyDict_Check(dict));
    assert (change);
    PyDictProxy* self = reinterpret_cast<PyDictProxy*>(
        TYPE.tp_alloc(&TYPE, 0));
    if (self)
    {
        self->dict_ = Python::newRef(dict);
        self->context_ = Python::newRef(context);
        self->change_ = change;
    }
    return self;
}

void PyDictProxy::dealloc(PyObject* self_)
{
    PyDictProxy* self = cast(self_);
    Py_XDECREF(self->dict_);
    Py_XDECREF(self->context_);
    Py_TYPE(self_)->tp_free(self_);
}

PyObject* PyDictProxy::iter(PyObject* selfObj)
{
    PyDictProxy* self = cast(selfObj);
    return PyObject_GetIter(self->dict_);
}

PyObject* PyDictProxy::repr(PyObject* selfObj)
{
    PyDictProxy* self = cast(selfObj);
    // Show as repr(underlying_dict) to look like a regular dict.
    return PyObject_Repr(self->dict_);
}

PyObject* PyDictProxy::str(PyObject* selfObj)
{
    PyDictProxy* self = cast(selfObj);
    return PyObject_Str(self->dict_);
}

PyObject* PyDictProxy::richcompare(PyObject* selfObj,
    PyObject* other, int op)
{
    PyDictProxy* self = cast(selfObj);
    return PyObject_RichCompare(self->dict_, other, op);
}

Py_ssize_t PyDictProxy::length(PyObject* selfObj)
{
    PyDictProxy* self = cast(selfObj);
    return PyDict_Size(self->dict_);
}

PyObject* PyDictProxy::getitem(PyObject* selfObj, PyObject* key)
{
    PyDictProxy* self = cast(selfObj);
    PyObject* value = PyDict_GetItemWithError(self->dict_, key);
    if (!value)
    {
        if (PyErr_Occurred()) return nullptr;
        PyErr_SetObject(PyExc_KeyError, key);
        return nullptr;
    }
    return Python::newRef(value);
}

int PyDictProxy::setitem(PyObject* selfObj, PyObject* key, PyObject* value)
{
    PyDictProxy* self = cast(selfObj);
    return self->change(key, value) ? 0 : -1;
}

// ===== Methods: read-only ====================================================

PyObject* PyDictProxy::_keys(PyObject* selfObj, PyObject* /*ignored*/)
{
    PyDictProxy* self = cast(selfObj);
    return PyDict_Keys(self->dict_);
}

PyObject* PyDictProxy::_values(PyObject* selfObj, PyObject* /*ignored*/)
{
    PyDictProxy* self = cast(selfObj);
    return PyDict_Values(self->dict_);
}

PyObject* PyDictProxy::_items(PyObject* selfObj, PyObject* /*ignored*/)
{
    PyDictProxy* self = cast(selfObj);
    return PyDict_Items(self->dict_);
}

PyObject* PyDictProxy::_get(PyObject* selfObj, PyObject* args)
{
    PyDictProxy* self = cast(selfObj);
    PyObject* key;
    PyObject* defaultValue = Py_None;

    if (!PyArg_UnpackTuple(args, "get", 1, 2, &key, &defaultValue))
    {
        return nullptr;
    }

    PyObject* value = PyDict_GetItemWithError(self->dict_, key);
    if (value) return Python::newRef(value);
    if (PyErr_Occurred()) return nullptr;
    return Python::newRef(defaultValue);
}

PyObject* PyDictProxy::_copy(PyObject* selfObj, PyObject* /*ignored*/)
{
    PyDictProxy* self = cast(selfObj);
    return PyDict_Copy(self->dict_);
}

// ===== Methods: mutating =====================================================

PyObject* PyDictProxy::_setdefault(PyObject* selfObj, PyObject* args)
{
    PyDictProxy* self = cast(selfObj);
    PyObject* key;
    PyObject* defaultValue = Py_None;

    if (!PyArg_UnpackTuple(args, "setdefault",
        1, 2, &key, &defaultValue))
    {
        return nullptr;
    }

    PyObject* existing = PyDict_GetItemWithError(self->dict_, key);
    if (existing) return Python::newRef(existing);
    if (PyErr_Occurred()) return nullptr;
    if (!self->change(key, defaultValue)) return nullptr;

    Py_INCREF(defaultValue);
    return defaultValue;
}

PyObject* PyDictProxy::_pop(PyObject* selfObj, PyObject* args)
{
    PyDictProxy* self = cast(selfObj);
    PyObject* key;
    PyObject* defaultValue = nullptr;

    if (!PyArg_UnpackTuple(args, "pop", 1, 2, &key, &defaultValue))
    {
        return nullptr;
    }

    PyObject* value = PyDict_GetItemWithError(self->dict_, key);
    if (!value)
    {
        if (PyErr_Occurred()) return nullptr;

        if (defaultValue)
        {
            Py_INCREF(defaultValue);
            return defaultValue;
        }

        PyErr_SetObject(PyExc_KeyError, key);
        return nullptr;
    }

    Py_INCREF(value);
    if (!self->change(key, nullptr))
    {
        Py_DECREF(value);
        return nullptr;
    }
    return value;
}

PyObject* PyDictProxy::_popitem(PyObject* selfObj, PyObject* args)
{
    (void)args;
    PyDictProxy* self = cast(selfObj);
    if (PyDict_Size(self->dict_) == 0)
    {
        PyErr_SetString(PyExc_KeyError,"popitem(): dictionary is empty");
        return nullptr;
    }

    Py_ssize_t pos = 0;
    PyObject* key;
    PyObject* value;

    if (!PyDict_Next(self->dict_, &pos, &key, &value))
    {
        PyErr_SetString(PyExc_RuntimeError, "popitem(): failed to get item");
        return nullptr;
    }

    PyObject* pair = PyTuple_Pack(2, key, value);
    if (!pair) return nullptr;

    if (!self->change(key, nullptr))
    {
        Py_DECREF(pair);
        return nullptr;
    }
    return pair;
}

PyObject* PyDictProxy::_clear(PyObject* selfObj, PyObject* /*ignored*/)
{
    PyDictProxy* self = cast(selfObj);

    PyObject* keys = PyDict_Keys(self->dict_);
    if (!keys) return nullptr;

    Py_ssize_t n = PyList_GET_SIZE(keys);
    for (Py_ssize_t i = 0; i < n; ++i)
    {
        PyObject* key = PyList_GET_ITEM(keys, i);  // borrowed
        if (!self->change(key, nullptr))
        {
            Py_DECREF(keys);
            return nullptr;
        }
    }
    Py_DECREF(keys);
    Py_RETURN_NONE;
}

PyObject* PyDictProxy::_update(PyObject* selfObj, PyObject* args,
    PyObject* kwargs)
{
    PyDictProxy* self = cast(selfObj);
    PyObject* other = nullptr;

    if (!PyArg_UnpackTuple(args, "update", 0, 1, &other))
    {
        return nullptr;
    }

    PyObject* merged = PyDict_New();
    if (!merged) return nullptr;

    if (other)
    {
        if (PyDict_Update(merged, other) < 0)
        {
            Py_DECREF(merged);
            return nullptr;
        }
    }

    if (kwargs)
    {
        if (PyDict_Update(merged, kwargs) < 0)
        {
            Py_DECREF(merged);
            return nullptr;
        }
    }

    Py_ssize_t pos = 0;
    PyObject* key;
    PyObject* value;

    while (PyDict_Next(merged, &pos, &key, &value))
    {
        if (!self->change(key, value))
        {
            Py_DECREF(merged);
            return nullptr;
        }
    }

    Py_DECREF(merged);
    Py_RETURN_NONE;
}

// ===== Methods: contains =====================================================

int PyDictProxy::_contains(PyObject* selfObj, PyObject* key)
{
    PyDictProxy* self = cast(selfObj);
    return PyDict_Contains(self->dict_, key);
}

PyMethodDef PyDictProxy::METHODS[] =
{
    {
        "keys",
        (PyCFunction)_keys,
        METH_NOARGS,
        "Return a new view of the dictionary's keys."
    },
    {
        "values",
        (PyCFunction)_values,
        METH_NOARGS,
        "Return a new view of the dictionary's values."
    },
    {
        "items",
        (PyCFunction)_items,
        METH_NOARGS,
        "Return a new view of the dictionary's items."
    },
    {
        "get",
        (PyCFunction)_get,
        METH_VARARGS,
        "Get value for key, or default if missing."
    },
    {
        "setdefault",
        (PyCFunction)_setdefault,
        METH_VARARGS,
        "Get value for key, inserting default if missing."
    },
    {
        "pop",
        (PyCFunction)_pop,
        METH_VARARGS,
        "Remove specified key and return the corresponding value."
    },
    {
        "popitem",
        (PyCFunction)_popitem,
        METH_VARARGS,
        "Remove and return an arbitrary (key, value) pair."
    },
    {
        "clear",
        (PyCFunction)_clear,
        METH_NOARGS,
        "Remove all items from the dictionary."
    },
    {
        "update",
        (PyCFunction)_update,
        METH_VARARGS | METH_KEYWORDS,
        "Update the dictionary from another mapping/iterable and keywords."
    },
    {
        "copy",
        (PyCFunction)_copy,
        METH_NOARGS,
        "Return a shallow copy of the underlying dict."
    },
    { nullptr, nullptr, 0, nullptr }
};

PyMappingMethods PyDictProxy::MAPPING_METHODS =
{
    &PyDictProxy::length,     // mp_length
    &PyDictProxy::getitem,    // mp_subscript
    &PyDictProxy::setitem     // mp_ass_subscript
};

PySequenceMethods PyDictProxy::SEQUENCE_METHODS =
{
    0,                        // sq_length
    0,                        // sq_concat
    0,                        // sq_repeat
    0,                        // sq_item
    0,                        // was_sq_slice
    0,                        // sq_ass_item
    0,                        // was_sq_ass_slice
    &PyDictProxy::_contains,  // sq_contains
    0,                        // sq_inplace_concat
    0                         // sq_inplace_repeat
};

PyTypeObject PyDictProxy::TYPE =
{
    .tp_name = "geodesk.DictProxy",
    .tp_basicsize = sizeof(PyDictProxy),
    .tp_dealloc = dealloc,
    .tp_repr = repr,
    .tp_as_sequence = &SEQUENCE_METHODS,
    .tp_as_mapping = &MAPPING_METHODS,
    .tp_str = str,
    .tp_flags = Py_TPFLAGS_DEFAULT, // | Py_TPFLAGS_DISALLOW_INSTANTIATION,
    .tp_doc = "DictProxy objects",
    .tp_richcompare = richcompare,
    .tp_iter = iter,
    .tp_methods = METHODS,
};
