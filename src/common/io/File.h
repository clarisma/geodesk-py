// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cstring>
#include <stdint.h>
#if defined(_WIN32)    
#ifndef NOMINMAX
// Prevent Windows headers from clobbering min/max
#define NOMINMAX
#endif
#include <windows.h>
#endif 

#include "IOException.h"

#if defined(_WIN32)     
typedef HANDLE FileHandle;
static FileHandle INVALID_FILE_HANDLE = INVALID_HANDLE_VALUE;
#else
typedef int FileHandle;
static FileHandle INVALID_FILE_HANDLE = -1;
#endif

class File 
{
public:
    enum OpenMode 
    {
        READ = 1 << 0,      // TODO: expected to stay stable
        WRITE = 1 << 1,     // TODO: expected to stay stable
        CREATE = 1 << 2,    // TODO: expected to stay stable    
        SPARSE = 1 << 3
    };

    File() = default;
    ~File() 
    {
        close();
    }

    FileHandle handle() const { return fileHandle_; }

    void open(const char* filename, int /* OpenMode */ mode);
    void close();
    bool isOpen() const { return fileHandle_ != INVALID_FILE_HANDLE; };

    uint64_t size() const;
    void setSize(uint64_t newSize);
    void expand(uint64_t newSize);
    void truncate(uint64_t newSize);

    void seek(uint64_t posAbsolute);
    size_t read(void* buf, size_t length);

    void force();

    /**
     * Returns the extension of the given filename (as pointer to ".ext"),
     * or an empty string if the filename does not have an extension.
     */
    static const char* extension(const char* filename, size_t len);
    static const char* extension(const char* filename)
    {
        return extension(filename, strlen(filename));
    }
    static const char* extension(std::string_view filename)
    {
        return extension(filename.data(), filename.length());
    }

protected:
#if defined(_WIN32) // Windows
    FileHandle fileHandle_ = INVALID_HANDLE_VALUE;
#elif defined(__linux__) || defined(__APPLE__) // Linux or MacOS
    FileHandle fileHandle_ = -1;
#endif
};



