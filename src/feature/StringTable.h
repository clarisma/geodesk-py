// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#ifdef GEODESK_PYTHON
#include <Python.h>
#endif
#include "types.h"

class StringTable
{
public:
    StringTable();
    ~StringTable();

    #ifdef GEODESK_PYTHON
    using HashCode = Py_hash_t;
    #else
    using HashCode = size_t;
    #endif

    void create(const uint8_t* pStrings);

    #ifdef GEODESK_PYTHON
    /**
     * Returns the Python string object representing the given
     * global-string code.
     *
     * @param code The global-string code.
     * @return A Python string object (new reference), or NULL if
     *         the string could not be created.
     */
    PyObject* getStringObject(int code);
    #endif
    GlobalString getGlobalString(int code);
    bool isValidCode(int code);
    int getCode(PyObject* strObj) const;
    int getCode(const char* str, int len) const;


private:
    struct Entry
    {
        uint32_t relPointer;
        uint32_t next;
    };

    int getCode(HashCode hash, const char* str, int len) const;

    uint32_t stringCount_;
    uint32_t lookupMask_;
    const uint8_t* stringBase_;
    uint8_t* arena_;
    uint16_t* buckets_;
    Entry* entries_;
    #ifdef GEODESK_PYTHON
    PyObject** stringObjects_;
    #endif
    // beware of alignment!
};

