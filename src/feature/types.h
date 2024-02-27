// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <Python.h>
#include <algorithm>
// #include <charconv>
#include <cstdint>
#include <cstring>
#include <string_view>
#include <common/util/pointer.h>

enum FeatureType
{
    NODE = 0,
    WAY = 1,
    RELATION = 2
};

enum FeatureIndexType
{
    NODES = 0,
    WAYS = 1,
    AREAS = 2,
    RELATIONS = 3
};

class IndexBits
{
public:
    static uint32_t fromCategory(int category)
    {
        return category == 0 ? 0 : (1 << (category - 1));
    }
};

enum FeatureFlags
{
    LAST_SPATIAL_ITEM = 1,
    AREA = 1 << 1,
    RELATION_MEMBER = 1 << 2,
    WAYNODE = 1 << 5,
    MULTITILE_WEST = 1 << 6,
    MULTITILE_NORTH = 1 << 7
};

class FeatureTypes
{
public:
    static const uint32_t NODES =     0b00000000'00000101'00000000'00000101;
    static const uint32_t WAYS =      0b00000000'11110000'00000000'11110000;
    static const uint32_t RELATIONS = 0b00001111'00000000'00001111'00000000;
    static const uint32_t AREAS =     0b00001010'10100000'00001010'10100000;
    static const uint32_t NONAREA_WAYS = WAYS & (~AREAS);
    static const uint32_t NONAREA_RELATIONS = RELATIONS & (~AREAS);
    static const uint32_t ALL = NODES | WAYS | RELATIONS;
    static const uint32_t WAYNODE_FLAGGED = 0b00000000'11110101'00000000'00000000;
    static const uint32_t RELATION_MEMBERS = 0b00001100'11000100'00001100'11000100;

    constexpr FeatureTypes(uint32_t types) : types_(types) {}
    FeatureTypes operator|(FeatureTypes other) const 
    { 
        return FeatureTypes(types_ | other.types_);
    }

    FeatureTypes& operator|=(const FeatureTypes& other) 
    {
        types_ |= other.types_;
        return *this;
    }

    FeatureTypes operator&(FeatureTypes other) const
    {
        return FeatureTypes(types_ & other.types_);
    }

    FeatureTypes operator&(uint32_t other) const
    {
        return FeatureTypes(types_ & other);
    }

    FeatureTypes& operator&=(const FeatureTypes& other)
    {
        types_ &= other.types_;
        return *this;
    }

    operator uint32_t() const
    {
        return types_;
    }

    inline bool acceptFlags(int flags) const
    {
        // unlike Java, it is not a given that shift will only consider the
        // lowest 5 bits, so we'll apply a mask
        uint32_t typeFlag = 1 << ((flags >> 1) & 0x1f);
        return (types_ & typeFlag) != 0;
    }

private:
    uint32_t types_;
};


class Tip
{
public:
    Tip(uint32_t tip) : tip_(tip) {}

    operator uint32_t() const
    {
        return tip_;
    }

    Tip& operator+=(int delta)
    {
        tip_ += delta;
        return *this;
    }


private:
    uint32_t tip_;
};

enum MemberFlags
{
    LAST = 1,
    FOREIGN = 2,
    DIFFERENT_ROLE = 4,
    DIFFERENT_TILE = 8
};

namespace FeatureConstants
{
    static const Tip START_TIP(0x4000);
    static const int MAX_COMMON_KEY = (1 << 13) - 2;
    static const int MAX_COMMON_ROLE = (1 << 15) - 1;
};


class GlobalString
{
public:
    GlobalString(const uint8_t* p) { ptr_ = p; }
    GlobalString(pointer p) { ptr_ = p; }

    const uint8_t* asBytePointer() const { return ptr_; }

    // TODO: This will change if stringtable format is updated
    int length() const
    {
        int len = *ptr_;
        int lenHi = *(ptr_ + 1);
        return (len & 128) ? ((lenHi << 7) | (len & 0x7f)) : len;
    }

    // TODO: This will change if stringtable format is updated
    const char* data() const
    {
        return reinterpret_cast<const char*>(ptr_ +
            ((*ptr_ & 128) ? 2 : 1));
    }

    bool equals(const char* str, size_t len) const
    {
        if (length() != len) return false;
        return std::memcmp(data(), str, len) == 0;
    }

    bool equals(std::string_view s) const
    {
        return equals(s.data(), s.length());
    }

    std::string_view toStringView() const
    {
        return std::string_view(data(), length());
    }

    #ifdef GEODESK_PYTHON
    PyObject* toStringObject() const
    {
        return PyUnicode_FromStringAndSize(data(), length());
    }
    #endif

    // TODO: std::from_chars not supported in GNU 10.2.1
    /*
    double toDouble(double defaultValue = 0.0)
    {
        const char* p = data();
        double v = defaultValue;
        std::from_chars(p, p + length(), v);
        return v;
    }
    */

    double toDouble(double defaultValue = 0.0)
    {
        const int BUFFER_SIZE = 32;
        char buf[BUFFER_SIZE];
        int len = std::min(length(), BUFFER_SIZE - 1);
        std::memcpy(buf, data(), len);
        buf[len] = 0;
        char* end;
        double v = strtod(buf, &end);
        return (end == buf) ? defaultValue : v;
    }

private:
    const uint8_t* ptr_;
};

// TODO: This may eventually differ:
// - Global strings will have a single-byte len (max. 255 bytes)
// - Local strings have a variable-byte len (max. 16K bytes)
// - Currently, Global strings use same encoding as local
// No! Don't change this, compiled matcher assumes local and global strings
// have same format. (But could require globals to be 127 char max)
typedef GlobalString LocalString;

namespace TagValue
{
    static const int MIN_NUMBER = -256;
    static const int MAX_WIDE_NUMBER = (1 << 30) - 1 + MIN_NUMBER;
    static const int MAX_NARROW_NUMBER = (1 << 16) - 1 + MIN_NUMBER;

    static const double SCALE_FACTORS[] = { 1.0, 0.1, 0.01, 0.001 };

    static inline double doubleFromWideNumber(uint32_t val)
    {
        double mantissa = (int32_t)(val >> 2) + MIN_NUMBER;
        int scale = val & 3;
        return mantissa * SCALE_FACTORS[scale];
    }
}