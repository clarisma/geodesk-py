// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <Python.h>
#include <algorithm>
// #include <charconv>
#include <cstdint>
#include <cstring>
#include <string_view>
#include <common/math/Decimal.h>
#include <common/util/pointer.h>
#include "FeatureTypes.h"
#include "TagValue.h"
#include "Tip.h"

// TODO: make enum class
// TODO: move to public API
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


enum MemberFlags
{
    LAST = 1,
    FOREIGN = 2,
    DIFFERENT_ROLE = 4,
    DIFFERENT_TILE = 8,     
        // TODO: This will change in 2.0 for relation tables and feature-node tables
        // (moves to Bit 2 == value 4, to accommodate more TEX bits)
    WIDE_NODE_TEX = 8,
    WIDE_RELATION_TEX = 8,
    WIDE_MEMBER_TEX = 16,
};

namespace FeatureConstants
{
    static const Tip START_TIP(0x4000);     
        // TODO: move to Tip? No, not really a characteriastic of TIP,
        // it is driven by the encoding used by member/node/relation tables
    static const int MAX_COMMON_KEY = (1 << 13) - 2;
    static const int MAX_COMMON_ROLE = (1 << 15) - 1;
};

/*
// TODO: Replace with ShortVarString
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
*/