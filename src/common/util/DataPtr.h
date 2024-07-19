// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include <cstdint>
#include <cstring>
#include <iostream>

// TODO: Should Byte be signed?

class DataPtr
{
public:
    DataPtr() noexcept : p_(nullptr) {}
    DataPtr(void* p) noexcept : p_(reinterpret_cast<uint8_t*>(p)) {}
    DataPtr(const DataPtr& other) noexcept : p_(other.p_) {}
    
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

    DataPtr operator+(std::ptrdiff_t offset) const noexcept
    {
        return DataPtr(p_ + offset);
    }

    DataPtr operator-(std::ptrdiff_t offset) const noexcept
    {
        return DataPtr(p_ - offset);
    }

    DataPtr& operator+=(std::ptrdiff_t offset) noexcept
    {
        p_ += offset;
        return *this;
    }

    DataPtr& operator-=(std::ptrdiff_t offset) noexcept
    {
        p_ -= offset;
        return *this;
    }

    bool operator!() const noexcept
    {
        return p_ == nullptr;
    }

    void putByte(uint8_t value) noexcept
    {
        *reinterpret_cast<uint8_t*>(p_) = value;
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

    DataPtr follow() const noexcept
    {
        return DataPtr(p_ + getInt());
    }

private:
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

    uint8_t* p_;
};
