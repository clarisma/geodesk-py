// Copyright (c) 2026 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

// Copyright (c) 2026 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "PythonPtr.h"

#include <Python.h>
#include <atomic>
#include <utility>

/// \brief A thread-safe, lazily initialized Python object reference.
///
/// Owns a strong reference to a Python object, initialized on first use.
/// Initialization uses compare-and-exchange, allowing concurrent threads
/// to race to create the object. Only one reference is published; any
/// losing candidate is automatically released.
///
/// Once initialized, get() requires only an atomic load.
///
/// The cached reference remains valid until the CachedPythonRef is
/// destroyed. Destruction must not overlap with concurrent access.
///
/// All Python API operations require an attached Python thread state.
///
class CachedPythonRef
{
public:
    /// Constructs an empty cache.
    CachedPythonRef() noexcept = default;

    CachedPythonRef(const CachedPythonRef&) = delete;
    CachedPythonRef& operator=(const CachedPythonRef&) = delete;

    CachedPythonRef(CachedPythonRef&&) = delete;
    CachedPythonRef& operator=(CachedPythonRef&&) = delete;

    /// Releases the cached reference.
    /// Must not race with readers or initializers.
    ~CachedPythonRef()
    {
        // TODO: Don't destroy for now, need to address lifetime
        // //  of Environemnt class (where CachedPythonRef is used)
        // Py_XDECREF(obj_.load(std::memory_order_relaxed));
    }

    /// Returns the cached object as a borrowed reference.
    /// @return Cached object, or nullptr if not initialized.
    PyObject* get() const noexcept
    {
        return obj_.load(std::memory_order_acquire);
    }

    /// Returns the cached object, initializing it if necessary.
    /// @param factory Callable returning a PythonPtr with a new reference.
    /// @return Borrowed reference, or nullptr on failure.
    ///
    /// The factory may be called by multiple concurrent threads.
    /// Only the winning object is retained.
    ///
    /// The factory must set a Python exception if it fails.
    template<typename Factory>
    PyObject* getOrCreate(Factory&& factory)
    {
        PyObject* obj = get();
        if (obj) [[likely]] return obj;

        PythonPtr candidate(std::forward<Factory>(factory)());
        if (!candidate) return nullptr;

        PyObject* expected = nullptr;

        if (obj_.compare_exchange_strong(
            expected,
            candidate.get(),
            std::memory_order_release,
            std::memory_order_acquire))
        {
            return candidate.release();
        }

        // Another thread published its object first.
        // candidate releases its reference automatically.
        return expected;
    }

    /// Returns the cached object, creating it with a member function.
    /// @param instance Object on which to invoke the factory.
    /// @param factory Member function returning a new Python reference.
    /// @return Borrowed reference, or nullptr on failure.
    ///
    template<typename T>
    PyObject* getOrCreate(T* instance, PyObject* (T::*factory)())
    {
        return getOrCreate([instance, factory]() -> PyObject*
        {
            return (instance->*factory)();
        });
    }

private:
    std::atomic<PyObject*> obj_{nullptr};
};