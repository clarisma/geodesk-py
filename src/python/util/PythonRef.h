// Copyright (c) 2025 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <Python.h>
#include <type_traits>

/// @brief Owning reference to a PyObject* (RAII, shared semantics)
///
template<typename T>
class PythonRef
{
    static_assert(std::is_base_of_v<PyObject, T>,
        "PythonRef<T> requires T to derive from PyObject");

public:
    PythonRef() noexcept : obj_(nullptr) {}

    /// @brief Takes ownership of a new reference.
    /// @param obj New reference (will be DECREF'ed on destruction)
    ///
    explicit PythonRef(T *obj) noexcept :
        obj_(obj)
    {
    }

    /// @brief Copy-constructs, incrementing the reference count.
    /// @param other Other PythonRef to copy from
    ///
    PythonRef(const PythonRef &other) noexcept :
        obj_(other.obj_)
    {
        Py_XINCREF(obj_);
    }

    /// @brief Move-constructs, stealing the reference.
    /// @param other Other PythonRef to move from
    ///
    PythonRef(PythonRef &&other) noexcept :
        obj_(other.obj_)
    {
        other.obj_ = nullptr;
    }

    /// @brief Destroys the reference, decrementing the refcount
    ///
    ~PythonRef()
    {
        Py_XDECREF(obj_);
    }

    /// @brief Copy-assigns, adjusting reference counts appropriately.
    /// @param other Other PythonRef to copy from.
    /// @return *this
    PythonRef &operator=(const PythonRef &other) noexcept
    {
        PyObject *newObj = other.obj_;
        Py_XINCREF(newObj);
        PyObject *oldObj = obj_;
        obj_ = newObj;
        Py_XDECREF(oldObj);
        return *this;
    }

    /// @brief Move-assigns, stealing the reference.
    /// @param other Other PythonRef to move from.
    /// @return *this
    PythonRef &operator=(PythonRef &&other) noexcept
    {
        if (this != &other)
        {
            T *oldObj = obj_;
            obj_ = other.obj_;
            other.obj_ = nullptr;
            Py_XDECREF(oldObj);
        }
        return *this;
    }

    /// @brief Gets the underlying PyObject*.
    /// @return Raw pointer; may be nullptr.
    T *get() const noexcept
    {
        return obj_;
    }

    /// @brief Bool conversion; true if non-null.
    explicit operator bool() const noexcept
    {
        return obj_ != nullptr;
    }

    /// @brief Releases ownership without DECREF.
    /// @return Raw pointer; caller now owns the reference.
    T *release() noexcept
    {
        T *tmp = obj_;
        obj_ = nullptr;
        return tmp;
    }

    /// @brief Resets to a new object, updating reference counts.
    /// @param obj New reference (may be nullptr).
    ///
    void reset(T *obj = nullptr) noexcept
    {
        Py_XINCREF(obj);
        T *oldObj = obj_;
        obj_ = obj;
        Py_XDECREF(oldObj);
    }

    /// @brief Member access to the underlying PyObject*.
    /// @return Raw pointer; may be nullptr.
    ///
    T *operator->() const noexcept
    {
        return obj_;
    }

    /// @brief Implicitly convert to PyObject* (borrowed reference).
    /// @return Raw pointer; may be nullptr
    ///
    operator PyObject *() const noexcept    // NOLINT implicit
    {
        return static_cast<PyObject*>(obj_);
    }

private:
    T *obj_;
};

using PyObjectRef = PythonRef<PyObject>;