// Copyright (c) 2025 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <Python.h>

/// @brief Proxy around a Python dict that forwards reads and delegates
///     all mutations to a callback.
///
class PyDictProxy : public PyObject
{
public:
    /// @brief Signature of the change callback.
    /// @param context Arbitrary context object.
    /// @param dict The wrapped dict (owned elsewhere).
    /// @param key Key being changed.
    /// @param value New value, or nullptr to delete the key.
    /// @return true on success, false on veto/error (with exception set).
    using ChangeItemFunc = bool (*)(PyObject* context, PyObject* dict,
        PyObject* key, PyObject* value);

    static PyTypeObject TYPE;
    static PyMethodDef METHODS[];
    static PyMappingMethods MAPPING_METHODS;
    static PySequenceMethods SEQUENCE_METHODS;  // only __contains__

    /// @brief Create a new DictProxy wrapping an existing dict.
    /// @param dict The dict to wrap (must be a real dict).
    /// @param context Context passed to the callback (may be nullptr).
    /// @param change Callback to perform mutations (must not be nullptr).
    /// @return New reference, or nullptr on error.
    static PyDictProxy* create(PyObject* dict, PyObject* context,
        ChangeItemFunc change);

    static void dealloc(PyObject* self);
    static PyObject* iter(PyObject* self);
    static PyObject* repr(PyObject* self);
    static PyObject* str(PyObject* self);
    static PyObject* richcompare(PyObject* self, PyObject* other, int op);
    static Py_ssize_t length(PyObject* self);
    static PyObject* getitem(PyObject* self, PyObject* key);
    static int setitem(PyObject* self, PyObject* key, PyObject* value);

private:
    bool change(PyObject* key, PyObject* value)
    {
        bool res = change_(context_, dict_, key, value);
        assert(res || PyErr_Occurred());
            // callback must raise error if returning false
        return res;
    }

    /// @brief dict.keys()
    static PyObject* _keys(PyObject* self, PyObject* ignored);

    /// @brief dict.values()
    static PyObject* _values(PyObject* self, PyObject* ignored);

    /// @brief dict.items()
    static PyObject* _items(PyObject* self, PyObject* ignored);

    /// @brief dict.get(key, default=None)
    static PyObject* _get(PyObject* self, PyObject* args);

    /// @brief dict.setdefault(key, default=None)
    static PyObject* _setdefault(PyObject* self, PyObject* args);

    /// @brief dict.pop(key[, default])
    static PyObject* _pop(PyObject* self, PyObject* args);

    /// @brief dict.popitem()
    static PyObject* _popitem(PyObject* self, PyObject* args);

    /// @brief dict.clear()
    static PyObject* _clear(PyObject* self, PyObject* ignored);

    /// @brief dict.update([other], **kwds)
    static PyObject* _update(
        PyObject* self,
        PyObject* args,
        PyObject* kwargs);

    /// @brief dict.copy() -> real dict
    static PyObject* _copy(PyObject* self, PyObject* ignored);

    /// @brief __contains__(key)
    static int _contains(PyObject* self, PyObject* key);

    /// @brief Helper: cast PyObject* to PyDictProxy*.
    static PyDictProxy* cast(PyObject* self)
    {
        return reinterpret_cast<PyDictProxy*>(self);
    }

    PyObject* dict_;
    PyObject* context_;
    ChangeItemFunc change_;
};

