// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cstdint>
#include <cstring>
#include <iostream>

// TODO: Should Byte be signed?

class DataPtr
{
public:
    DataPtr() noexcept : p_(nullptr) {}
    DataPtr(uint8_t* p) noexcept : p_(p) {}
    DataPtr(uintptr_t p) noexcept : p_(reinterpret_cast<uint8_t*>(p)) {}
    DataPtr(const uint8_t* p) noexcept : p_(const_cast<uint8_t*>(p)) {}
    DataPtr(const DataPtr& other) noexcept : p_(other.p_) {}

    DataPtr& operator=(uint8_t* p) noexcept
    {
        p_ = p;
        return *this;
    }

    // Non-const assignment operator for const pointers
    DataPtr& operator=(const uint8_t* p) noexcept
    {
        p_ = const_cast<uint8_t*>(p);
        return *this;
    }

    // Deleted const assignment operators to prevent assigning non-const pointers
    // to const DataPtr objects
    DataPtr& operator=(uint8_t* p) const noexcept = delete;

    uint8_t getByte() const noexcept
    {
        return *reinterpret_cast<uint8_t*>(p_);
    }

    int16_t getShort() const noexcept
    {
        return *reinterpret_cast<int16_t*>(p_);
    }

    uint16_t getUnsignedShort() const noexcept
    {
        return *reinterpret_cast<uint16_t*>(p_);
    }

    int32_t getInt() const noexcept
    {
        return *reinterpret_cast<int32_t*>(p_);
    }

    uint32_t getUnsignedInt() const noexcept
    {
        return *reinterpret_cast<uint32_t*>(p_);
    }

    int64_t getLong() const noexcept
    {
        return *reinterpret_cast<int64_t*>(p_);
    }

    uint64_t getUnsignedLong() const noexcept
    {
        return *reinterpret_cast<uint64_t*>(p_);
    }

    float getFloat() const noexcept
    {
        return *reinterpret_cast<float*>(p_);
    }

    double getDouble() const noexcept
    {
        return *reinterpret_cast<double*>(p_);
    }

    int16_t getShortUnaligned() const noexcept
    {
        return getUnaligned<int16_t>(p_);
    }

    uint16_t getUnsignedShortUnaligned() const noexcept
    {
        return getUnaligned<uint16_t>(p_);
    }

    int32_t getIntUnaligned() const noexcept
    {
        return getUnaligned<int32_t>(p_);
    }

    uint32_t getUnsignedIntUnaligned() const noexcept
    {
        return getUnaligned<uint32_t>(p_);
    }

    int64_t getLongUnaligned() const noexcept
    {
        return getUnaligned<int64_t>(p_);
    }

    uint64_t getUnsignedLongUnaligned() const noexcept
    {
        return getUnaligned<uint64_t>(p_);
    }

    float getFloatUnaligned() const noexcept
    {
        return getUnaligned<float>(p_);
    }

    double getDoubleUnaligned() const noexcept
    {
        return getUnaligned<double>(p_);
    }

    // Implicit conversions to pointer types
    operator uintptr_t () const noexcept { return reinterpret_cast<uintptr_t>(p_); }
    operator int8_t* () const noexcept { return reinterpret_cast<int8_t*>(p_); }
    operator uint8_t* () const noexcept { return reinterpret_cast<uint8_t*>(p_); }
    operator int16_t* () const noexcept { return reinterpret_cast<int16_t*>(p_); }
    operator uint16_t* () const noexcept { return reinterpret_cast<uint16_t*>(p_); }
    operator int32_t* () const noexcept { return reinterpret_cast<int32_t*>(p_); }
    operator uint32_t* () const noexcept { return reinterpret_cast<uint32_t*>(p_); }
    operator int64_t* () const noexcept { return reinterpret_cast<int64_t*>(p_); }
    operator uint64_t* () const noexcept { return reinterpret_cast<uint64_t*>(p_); }
    operator float* () const noexcept { return reinterpret_cast<float*>(p_); }
    operator double* () const noexcept { return reinterpret_cast<double*>(p_); }
    operator const void* () const noexcept { return reinterpret_cast<const void*>(p_); }
    operator const int8_t* () const noexcept { return reinterpret_cast<const int8_t*>(p_); }
    operator const uint8_t* () const noexcept { return reinterpret_cast<const uint8_t*>(p_); }
    operator const int16_t* () const noexcept { return reinterpret_cast<const int16_t*>(p_); }
    operator const uint16_t* () const noexcept { return reinterpret_cast<const uint16_t*>(p_); }
    operator const int32_t* () const noexcept { return reinterpret_cast<const int32_t*>(p_); }
    operator const uint32_t* () const noexcept { return reinterpret_cast<const uint32_t*>(p_); }
    operator const int64_t* () const noexcept { return reinterpret_cast<const int64_t*>(p_); }
    operator const uint64_t* () const noexcept { return reinterpret_cast<const uint64_t*>(p_); }
    operator const float* () const noexcept { return reinterpret_cast<const float*>(p_); }
    operator const double* () const noexcept { return reinterpret_cast<const double*>(p_); }

    DataPtr operator+(std::ptrdiff_t offset) const noexcept
    {
        return DataPtr(p_ + offset);
    }

    DataPtr operator+(int offset) const noexcept
    {
        return DataPtr(p_ + offset);
    }

    DataPtr operator+(unsigned int offset) const noexcept
    {
        return DataPtr(p_ + offset);
    }

    DataPtr operator-(std::ptrdiff_t offset) const noexcept
    {
        return DataPtr(p_ - offset);
    }

    DataPtr operator-(int offset) const noexcept
    {
        return DataPtr(p_ - offset);
    }

    DataPtr operator-(unsigned int offset) const noexcept
    {
        return DataPtr(p_ - offset);
    }

    DataPtr& operator+=(std::ptrdiff_t offset) noexcept
    {
        p_ += offset;
        return *this;
    }

    DataPtr& operator+=(int offset) noexcept
    {
        p_ += offset;
        return *this;
    }

    DataPtr& operator+=(unsigned int offset) noexcept
    {
        p_ += offset;
        return *this;
    }

    DataPtr& operator-=(std::ptrdiff_t offset) noexcept
    {
        p_ -= offset;
        return *this;
    }

    DataPtr& operator-=(int offset) noexcept
    {
        p_ -= offset;
        return *this;
    }

    DataPtr& operator-=(unsigned int offset) noexcept
    {
        p_ -= offset;
        return *this;
    }

    std::ptrdiff_t operator-(const DataPtr& other) const noexcept
    {
        return p_ - other.p_;
    }

    std::ptrdiff_t operator-(const uint8_t* other) const noexcept
    {
        return p_ - other;
    }

    bool operator!() const noexcept
    {
        return p_ == nullptr;
    }

    DataPtr follow() const noexcept
    {
        return DataPtr(p_ + getInt());
    }

    DataPtr followUnaligned() const noexcept
    {
        return DataPtr(p_ + getIntUnaligned());
    }


protected:
    template<typename T>
    static T getUnaligned(const uint8_t* p) noexcept
    {
#if defined(__GNUC__) || defined(__clang__)
        T value;
        std::memcpy(&value, p, sizeof(value));
        return value;
#elif defined(_MSC_VER)
        return *reinterpret_cast<const T __unaligned*>(p);
#else
        T value;
        std::memcpy(&value, p, sizeof(value));
        return value;
#endif
    }

    uint8_t* p_;
};
