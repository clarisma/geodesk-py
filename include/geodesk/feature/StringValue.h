// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <common/util/Strings.h>

namespace geodesk {

/// @brief A string pointer (used for keys, values and roles).
///
/// Converts implicitly to `std::string` and `std::string_view`.
///
/// **Warning**: A StringValue object is a lightweight wrapper
/// around a pointer to string data, contained in a FeatureStore.
/// It becomes invalid once that FeatureStore
/// is closed. To use its value beyond the lifetime of its
/// FeatureStore, assign it to a `std::string` (which
/// allocates its own copy of the string).
///
class StringValue 
{
public:
    StringValue() : str_(reinterpret_cast<const ShortVarString*>("")) {}
        // legal (0-char is interpreted as "size 0"), but ugly

    // NOLINTNEXTLINE(google-explicit-constructor)
    StringValue(const ShortVarString* str) : str_(str) {}
    explicit StringValue(const uint8_t* bytes) :
        str_(reinterpret_cast<const ShortVarString*>(bytes)) {}

    /// Pointer to the characters of the string.
    ///
    /// @return Pointer to the string's bytes (UTF-8 encoded, *not* null-terminated)
    ///
    const char* data() const noexcept { return str_->data(); }

    /// @brief `true` if the length of the string is `0`
    ///
    bool isEmpty() const noexcept { return str_->isEmpty(); }

    /// @brief The length of the string
    ///
    /// @return number of bytes in the string (*not* characters)
    size_t size() const noexcept { return str_->length(); }

    bool operator==(const std::string_view sv) const noexcept
    {
        return *str_ == sv;
    }
    bool operator!=(const std::string_view sv) const noexcept
    {
        return *str_ != sv;
    }

    /// The string as a `std::string_view`
    ///
    /// **Warning:** This `string_view` becomes invalid once the
    /// FeatureStore containing its data is closed.
    ///
    // NOLINTNEXTLINE(google-explicit-constructor)
    operator std::string_view() const noexcept { return str_->toStringView(); }

    /// Creates this string as a new `std::string`.
    ///
    // NOLINTNEXTLINE(google-explicit-constructor)
    operator std::string() const { return str_->toString(); }       // may throw

    /// Pointer to the raw data representing this string.
    ///
    /// **Warning:** This pointer becomes invalid once the
    /// FeatureStore containing its data is closed.
    ///
    const uint8_t* rawBytes() const { return str_->rawBytes(); }

private:
    const ShortVarString* str_;
};

template<typename Stream>
Stream& operator<<(Stream& out, const StringValue& s)
{
    std::string_view sv = s; // static_cast<const std::string_view&>(s);
    out.write(sv.data(), sv.size());
    return out;
}

} // namespace geodesk

template<>
struct std::hash<geodesk::StringValue>
{
    size_t operator()(const geodesk::StringValue& s) const noexcept
    {
        std::string_view sv = s;
        return Strings::hash(sv.data(), sv.size());
    }
};
