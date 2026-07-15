// Copyright (c) 2026 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <Python.h>
#include <utility>

/// \brief A smart pointer that owns a reference to a PyObject.
///
/// The constructor *steals* a reference (i.e. takes ownership of a
/// new reference, such as one returned by PyObject_GetAttr or
/// PyLong_FromLong). To wrap a *borrowed* reference (e.g. an argument
/// passed into a C function, or the result of PyDict_GetItem), use
/// PythonPtr::borrow(), which increments the refcount.
///
/// The destructor releases the reference. Copying increments the
/// refcount; moving transfers ownership. All operations are
/// null-safe (Py_XINCREF / Py_XDECREF).
///
class PythonPtr
{
public:
    /// Constructs an empty pointer.
    PythonPtr() noexcept : obj_(nullptr) {}

    /// Takes ownership of a new (strong) reference. Does not incref.
    explicit PythonPtr(PyObject* obj) noexcept : obj_(obj) {}

    /// Wraps a borrowed reference, incrementing its refcount.
    static PythonPtr borrow(PyObject* obj) noexcept
    {
        Py_XINCREF(obj);
        return PythonPtr(obj);
    }

    PythonPtr(const PythonPtr& other) noexcept : obj_(other.obj_)
    {
        Py_XINCREF(obj_);
    }

    PythonPtr(PythonPtr&& other) noexcept : obj_(other.obj_)
    {
        other.obj_ = nullptr;
    }

    ~PythonPtr()
    {
        Py_XDECREF(obj_);
    }

    PythonPtr& operator=(const PythonPtr& other) noexcept
    {
        // Incref before decref so self-assignment (and assignment
        // between pointers to the same object) is safe
        Py_XINCREF(other.obj_);
        Py_XDECREF(obj_);
        obj_ = other.obj_;
        return *this;
    }

    PythonPtr& operator=(PythonPtr&& other) noexcept
    {
        if (this != &other)
        {
            Py_XDECREF(obj_);
            obj_ = other.obj_;
            other.obj_ = nullptr;
        }
        return *this;
    }

    /// Returns the raw pointer without affecting ownership.
    PyObject* get() const noexcept { return obj_; }

    /// Releases ownership and returns the raw pointer without
    /// decrementing the refcount. Use when passing the reference to
    /// an API that steals it (e.g. PyTuple_SET_ITEM) or when
    /// returning it from a C function.
    PyObject* release() noexcept
    {
        PyObject* obj = obj_;
        obj_ = nullptr;
        return obj;
    }

    /// Drops the current reference (if any) and takes ownership of
    /// `obj` (a new reference; does not incref).
    void reset(PyObject* obj = nullptr) noexcept
    {
        PyObject* old = obj_;
        obj_ = obj;
        Py_XDECREF(old);
    }

    void swap(PythonPtr& other) noexcept
    {
        std::swap(obj_, other.obj_);
    }

    explicit operator bool() const noexcept { return obj_ != nullptr; }

    bool operator==(const PythonPtr& other) const noexcept
    {
        return obj_ == other.obj_;
    }

    bool operator!=(const PythonPtr& other) const noexcept
    {
        return obj_ != other.obj_;
    }

private:
    PyObject* obj_;
};

inline void swap(PythonPtr& a, PythonPtr& b) noexcept
{
    a.swap(b);
}