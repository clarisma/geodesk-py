// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cstdint>
#include <iostream>
#include <type_traits>

template <typename T>
class PunnedPtr
{
    static_assert(alignof(T) >= 2, "Pointer must be at least 2-byte aligned");

public:
    PunnedPtr() noexcept : data_(0) {}
    PunnedPtr(T* ptr) noexcept : data_(reinterpret_cast<uintptr_t>(ptr)) {}
    PunnedPtr(uint64_t value) noexcept : data_(
        reinterpret_cast<uintptr_t>((value << 1) | 1)) {}

    bool isPointer() const noexcept { return (data_ & 1) == 0; }
    bool isNumber() const noexcept { return (data_ & 1) != 0; }
    T* pointer() const noexcept 
    { 
        assert(isPointer());
        return reinterpret_cast<T*>(data_); 
    }
    uint64_t number() const noexcept 
    { 
        assert(isNumber());
        return reinterpret_cast<uint64_t>(data_) >> 1;
    }

    bool operator==(const PunnedPtr<T>& other) { return data_ == other.data_; }
    operator T*() const noexcept { return pointer(); }
    operator uint64_t() const noexcept { return number(); }
    
private:
    uintptr_t data_;
};

