// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "File.h"

template<typename T> class MappedSegment;


class MappedFile : public File
{
public:
    ~MappedFile()
    {
        close();
    }

    // TODO: Ensure these are the same values as OpenMode
    enum MappingMode
    {
        READ = 1 << 0,
        WRITE = 1 << 1,
    };

    void* map(uint64_t offset, uint64_t length, int /* MappingMode */ mode);

    template<typename T>
    MappedSegment<T> mapSegment(uint64_t offset, uint64_t length)
    {
        return MappedSegment(map(offset, length, MappingMode::READ || MappingMode::WRITE));
    }

    template<typename T>
    MappedSegment<const T> mapReadonlySegment(uint64_t offset, uint64_t length)
    {
        return MappedSegment(map(offset, length, MappingMode::READ));
    }

    static void unmap(void* address, uint64_t length);
    void prefetch(void* address, uint64_t length);
        // TODO: technically, does not need to be part of MappedFile
    void sync(const void* address, uint64_t length);
};


template<typename T>
class MappedSegment
{
public:
    MappedSegment(T* ptr) :
        ptr_(ptr)
    {
    }

    MappedSegment(const MappedSegment&) = delete;

    ~MappedSegment()
    {
        release();
    }

    MappedSegment(MappedSegment&& other) noexcept :
        ptr_(other.ptr_),
        size_(other.size_)
    {
        other.ptr_ = nullptr;
        other.size_ = 0;
    }

    MappedSegment& operator=(MappedSegment&& other) noexcept
    {
        if (this != &other)
        {
            release();
            ptr_ = other.ptr_;
            size_ = other.size_;
            other.ptr_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    MappedSegment& operator=(const MappedSegment&) = delete;

    T* get() const noexcept { return ptr_; }
    size_t size() const noexcept { return size_; }
    void release()
    {
        if (ptr_)
        {
            MappedFile::unmap(ptr_, size_);
            ptr_ = nullptr;
            size_ = 0;
        }
    }

private:
    T* ptr_;
    size_t size_;
};

