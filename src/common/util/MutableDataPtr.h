// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cstdint>
#include <cstring>
#include <iostream>
#include "DataPtr.h"
// TODO: Should Byte be signed?

class MutableDataPtr : public DataPtr
{
public:
    MutableDataPtr() noexcept : DataPtr() {}
    MutableDataPtr(uint8_t* p) noexcept : DataPtr(p) {}
    MutableDataPtr(const MutableDataPtr& other) noexcept : DataPtr(other.p_) {}

    MutableDataPtr& operator=(uint8_t* p) noexcept
    {
        p_ = p;
        return *this;
    }
        
    void putByte(uint8_t value) noexcept
    {
        *reinterpret_cast<uint8_t*>(p_) = value;
    }

    void putBytes(const void* src, size_t size) noexcept
    {
        memcpy(p_, src, size);
    }

    void putShort(int16_t value) noexcept
    {
        *reinterpret_cast<int16_t*>(p_) = value;
    }

    void putUnsignedShort(uint16_t value) noexcept
    {
        *reinterpret_cast<uint16_t*>(p_) = value;
    }

    void putInt(int32_t value) noexcept
    {
        *reinterpret_cast<int32_t*>(p_) = value;
    }

    void putUnsignedInt(uint32_t value) noexcept
    {
        *reinterpret_cast<uint32_t*>(p_) = value;
    }

    void putLong(int64_t value) noexcept
    {
        *reinterpret_cast<int64_t*>(p_) = value;
    }

    void putUnsignedLong(uint64_t value) noexcept
    {
        *reinterpret_cast<uint64_t*>(p_) = value;
    }

    void putFloat(float value) noexcept
    {
        *reinterpret_cast<float*>(p_) = value;
    }

    void putDouble(double value) noexcept
    {
        *reinterpret_cast<double*>(p_) = value;
    }

    void putShortUnaligned(int16_t value) noexcept
    {
        putUnaligned<int16_t>(p_, value);
    }

    void putUnsignedShortUnaligned(uint16_t value) noexcept
    {
        putUnaligned<uint16_t>(p_, value);
    }

    void putIntUnaligned(int32_t value) noexcept
    {
        putUnaligned<int32_t>(p_, value);
    }

    void putUnsignedIntUnaligned(uint32_t value) noexcept
    {
        putUnaligned<uint32_t>(p_, value);
    }

    void putLongUnaligned(int64_t value) noexcept
    {
        putUnaligned<int64_t>(p_, value);
    }

    void putUnsignedLongUnaligned(uint64_t value) noexcept
    {
        putUnaligned<uint64_t>(p_, value);
    }

    void putFloatUnaligned(float value) noexcept
    {
        putUnaligned<float>(p_, value);
    }

    void putDoubleUnaligned(double value) noexcept
    {
        putUnaligned<double>(p_, value);
    }

    template<typename T>
    MutableDataPtr operator+(T offset) const noexcept
    {
        return MutableDataPtr(p_ + offset);
    }

    template<typename T>
    MutableDataPtr operator-(T offset) const noexcept
    {
        return MutableDataPtr(p_ - offset);
    }

    MutableDataPtr& operator+=(std::ptrdiff_t offset) noexcept
    {
        p_ += offset;
        return *this;
    }

    MutableDataPtr& operator-=(std::ptrdiff_t offset) noexcept
    {
        p_ -= offset;
        return *this;
    }

    std::ptrdiff_t operator-(const MutableDataPtr& other) const noexcept
    {
        return p_ - other.p_;
    }

private:
    template<typename T>
    static void putUnaligned(uint8_t* p, T value) noexcept
    {
#if defined(__GNUC__) || defined(__clang__)
        std::memcpy(p, &value, sizeof(value));
#elif defined(_MSC_VER)
        * reinterpret_cast<T __unaligned*>(p) = value;
#else
        std::memcpy(p, &value, sizeof(value));
#endif
    }
};
