// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cstring>
#include <filesystem>
#include <stdint.h>
#if defined(_WIN32)    
#ifndef NOMINMAX
// Prevent Windows headers from clobbering min/max
#define NOMINMAX
#endif
#include <windows.h>
#endif 

#include <common/alloc/Block.h>
#include "IOException.h"

#if defined(_WIN32)     
typedef HANDLE FileHandle;
static FileHandle INVALID_FILE_HANDLE = INVALID_HANDLE_VALUE;
#else
typedef int FileHandle;
static FileHandle INVALID_FILE_HANDLE = -1;
#endif

// TODO: Should methods be const?
//  Consider logical state, write(0 should not be const even thugh it does
//  not change the binary representation of the object

class File 
{
public:
    enum OpenMode 
    {
        READ = 1 << 0,      // TODO: expected to stay stable
        WRITE = 1 << 1,     // TODO: expected to stay stable
        CREATE = 1 << 2,    // TODO: expected to stay stable    
        SPARSE = 1 << 3,
        REPLACE_EXISTING = 1 << 4
    };

    File() = default;
    File(FileHandle handle) : fileHandle_(handle) {};
    File(const File& other) = delete;
    File(File&& other)
    {
        fileHandle_ = other.fileHandle_;
        other.fileHandle_ = INVALID_FILE_HANDLE;
    }

    ~File() 
    {
        close();
    }

    File& operator=(const File& other) = delete;
    File& operator=(File&& other) noexcept
    {
        if (this != &other)
        {
            fileHandle_ = other.fileHandle_;
            other.fileHandle_ = INVALID_FILE_HANDLE;
        }
        return *this;
    }

    
    FileHandle handle() const { return fileHandle_; }
    std::string fileName() const;

    void open(const char* filename, int /* OpenMode */ mode);
    void open(const std::filesystem::path& path, int /* OpenMode */ mode)
    {
        open(path.string().c_str(), mode);
    }

    void close();
    bool isOpen() const { return fileHandle_ != INVALID_FILE_HANDLE; };

    uint64_t size() const;
    void setSize(uint64_t newSize);
    void expand(uint64_t newSize);
    void truncate(uint64_t newSize);

    void seek(uint64_t posAbsolute);
    size_t read(void* buf, size_t length);
    size_t read(uint64_t ofs, void* buf, size_t length);
    size_t write(const void* buf, size_t length);

    void makeSparse();
    void allocate(uint64_t ofs, size_t length);
    void deallocate(uint64_t ofs, size_t length);
    void zeroFill(uint64_t ofs, size_t length);


    void force();

    void error(const char* what);

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

    static ByteBlock readAll(const char* filename);
    static ByteBlock readAll(const std::filesystem::path& path)
    {
        return readAll(path.string().c_str());
    }
    static void writeAll(const char* filename, const void* data, size_t size);
    static void writeAll(const std::filesystem::path& path, const void* data, size_t size)
    {
        writeAll(path.string().c_str(), data, size);
    }
    template<typename T>
    static void writeAll(const char* filename, T span)
    {
        writeAll(filename, span.data(), span.size());
    }
    template<typename T>
    static void writeAll(const std::filesystem::path& path, T span)
    {
        writeAll(path.string().c_str(), span);
    }
    
protected:
#if defined(_WIN32) // Windows
    FileHandle fileHandle_ = INVALID_HANDLE_VALUE;
#elif defined(__linux__) || defined(__APPLE__) // Linux or MacOS
    FileHandle fileHandle_ = -1;
#endif
};



