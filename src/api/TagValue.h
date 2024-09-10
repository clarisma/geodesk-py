// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <common/text/Format.h>
#include <common/compile/unreachable.h>
#include <common/math/Math.h>
#include "StringValue.h"
#include "feature/TagValue.h"

namespace geodesk {
/// @brief The value of a Tag. Converts implicitly to `std::string`,
/// `double`, `int` or `bool`.
///
/// - `double`: `NaN` if the string value does not start with a valid number
///
/// - `int`: `0` if the string value does not start with a valid number
///
/// - `bool`: `true` unless empty string, `"no"`, or a numerical value of `0`
///
/// **Warning**: A TagValue object is a lightweight wrapper
/// around a pointer to a Feature's tag data, contained
/// in a FeatureStore. It becomes invalid once that
/// FeatureStore is closed. To use its value beyond the
/// lifetime of the FeatureStore, convert it to a `std::string`,
/// which allocates a copy of the tag data (or one of the supported
/// scalar types).
///
/// @see Tag, Tags
///
class TagValue
{
public:
    TagValue() : taggedNumberValue_(1) {}   // empty string
    TagValue(uint64_t taggedNumberValue, StringValue str = StringValue()) :
        taggedNumberValue_(taggedNumberValue), stringValue_(str) {}

    // NOLINTNEXTLINE(google-explicit-constructor)
    operator std::string() const    // may throw
    {
        char buf[32];
        char *end;

        switch (type())
        {
        case 1:     // global string
        case 3:     // local string (fall through)
            return stringValue_;
        case 0:     // narrow number
            end = Format::integer(buf,
                ::TagValue::intFromNarrowNumber(rawNumberValue()));
            return std::string(buf, end-buf);
        case 2:     // wide number
            return ::TagValue::decimalFromWideNumber(rawNumberValue());
        default:
            UNREACHABLE_CASE
        }
    }

    // NOLINTNEXTLINE(google-explicit-constructor)
    operator double() const noexcept
    {
        double val;

        switch (type())
        {
        case 1:     // global string
        case 3:     // local string (fall through)
            Math::parseDouble(stringValue_, &val);
            return val;
        case 0:     // narrow number
            return ::TagValue::intFromNarrowNumber(rawNumberValue());
        case 2:     // wide number
            return ::TagValue::decimalFromWideNumber(rawNumberValue());
        default:
            UNREACHABLE_CASE
        }
    }

    // NOLINTNEXTLINE(google-explicit-constructor)
    operator int64_t() const noexcept
    {
        return static_cast<int64_t>(static_cast<double>(*this));
    }

    // NOLINTNEXTLINE(google-explicit-constructor)
    operator int() const noexcept
    {
        return static_cast<int>(static_cast<double>(*this));
    }

    // NOLINTNEXTLINE(google-explicit-constructor)
    operator bool() const noexcept
    {
        switch (type())
        {
        case 1:     // global string
        case 3:     // local string (fall through)
            return stringValue_.size() != 0 && stringValue_ != "no";
        case 0:     // narrow number
            return ::TagValue::intFromNarrowNumber(rawNumberValue()) != 0;
        case 2:     // wide number
            return ::TagValue::decimalFromWideNumber(rawNumberValue()) != 0;
        default:
            UNREACHABLE_CASE
        }
    }

    bool operator!() const noexcept
    {
        return !static_cast<bool>(*this);
    }

    bool operator==(const std::string_view &sv) const noexcept
    {
        char buf[32];
        char* end;
        switch (type())
        {
        case 1:     // global string
        case 3:     // local string (fall through)
            return stringValue_ == sv;
        case 0:     // narrow number
            end = Format::integer(buf,
                ::TagValue::intFromNarrowNumber(rawNumberValue()));
            return sv == std::string_view(buf, end-buf);
        case 2:     // wide number
            end = ::TagValue::decimalFromWideNumber(rawNumberValue()).format(buf);
            return sv == std::string_view(buf, end-buf);
        default:
            UNREACHABLE_CASE
        }
    }

    bool operator==(double val) const noexcept
    {
        return static_cast<double>(*this) == val;
    }

    bool operator==(int64_t val) const noexcept
    {
        return static_cast<int64_t>(*this) == val;
    }

    bool operator!=(const std::string_view& val) const noexcept
    {
        return !(*this == val);
    }

    bool operator!=(double val) const noexcept
    {
        return static_cast<double>(*this) != val;
    }

    bool operator!=(int64_t val) const noexcept
    {
        return !(*this == val);
    }

    bool operator<(const TagValue& other) const noexcept
    {
        return static_cast<int>(*this) < static_cast<int>(other);
    }

    // Define equality operator
    bool operator==(const TagValue& other) const noexcept
    {
        return static_cast<int>(*this) == static_cast<int>(other);
    }

    bool operator<(double val) const noexcept
    {
        return static_cast<double>(*this) < val;
    }

    bool operator>(double val) const noexcept
    {
        return static_cast<double>(*this) > val;
    }

    bool operator<=(double val) const noexcept
    {
        return static_cast<double>(*this) <= val;
    }

    bool operator>=(double val) const noexcept
    {
        return static_cast<double>(*this) >= val;
    }

    bool operator<(int64_t val) const noexcept
    {
        return static_cast<int64_t>(*this) < val;
    }

    bool operator>(int64_t val) const noexcept
    {
        return static_cast<int64_t>(*this) > val;
    }

    bool operator<=(int64_t val) const noexcept
    {
        return static_cast<int64_t>(*this) <= val;
    }

    bool operator>=(int64_t val) const noexcept
    {
        return static_cast<int64_t>(*this) >= val;
    }

    bool operator<(int val) const noexcept
    {
        return static_cast<int>(*this) < val;
    }

    bool operator>(int val) const noexcept
    {
        return static_cast<int>(*this) > val;
    }

    bool operator<=(int val) const noexcept
    {
        return static_cast<int>(*this) <= val;
    }

    bool operator>=(int val) const noexcept
    {
        return static_cast<int>(*this) >= val;
    }

private:
    int type() const { return taggedNumberValue_ & 3; }
    uint_fast32_t rawNumberValue() const
    {
        return static_cast<uint_fast32_t>(taggedNumberValue_ >> 2);
    }

    uint64_t taggedNumberValue_;
    StringValue stringValue_;

    template<typename Stream>
    friend Stream& operator<<(Stream& out, const TagValue& v);
};

template<typename Stream>
Stream& operator<<(Stream& out, const TagValue& v)
{
    switch (v.type())
    {
    case 1:     // global string
    case 3:     // local string (fall through)
        out << v.stringValue_;
        break;
    case 0:     // narrow number
    {
        char buf[32];
        char* end = Format::integer(buf,
            ::TagValue::intFromNarrowNumber(v.rawNumberValue()));
        out.write(buf, end - buf);
        break;
    }
    case 2:     // wide number
        out << ::TagValue::decimalFromWideNumber(v.rawNumberValue());
        break;
    default:
        UNREACHABLE_CASE
    }
    return out;
}

} // geodesk