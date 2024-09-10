// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <unordered_map>
#include <vector>
#include "Tag.h"

namespace geodesk {

/// @brief An object describing the key/value attributes
///   of a Feature.
///
/// **Warning**: A Tags object is a lightweight wrapper
/// around a pointer to a Feature's tag data, contained
/// in a FeatureStore. It becomes invalid once that
/// FeatureStore is closed. To use a Feature's keys and
/// values beyond the lifetime of the FeatureStore,
/// assign the Tags object to a `std::vector` or
/// `std::unordered_map` that whose type is `std::string`,
/// which allocates copies of the key/value data.
///
/// @see Tag, TagValue
///
class Tags 
{
public:
    /// @brief Looks up the tag value for the given key.
    ///
    /// @return the tag's value (or an empty string
    ///         if no tag with this key exists)
    TagValue operator[](std::string_view key) const noexcept;

    /// @brief Creates a map of keys to values.
    ///
    /// @param K  The key's type: StringValue, `std::string` or `std::string_view`
    /// @param V  The value's type: TagValue, `std::string`, `double`, `int` or `bool`
    ///
    template<typename K, typename V>
    operator std::unordered_map<K,V>() const // NOLINT(google-explicit-constructor)
    {
        std::unordered_map<K,V> map;
        // TODO: populate
        return map;
    }

    /// @brief Creates a vector containing the key/value pairs
    ///
    /// @param T  Tag or `std::pair<K,V>` (where `K` can be
    ///    StringValue, `std::string` or `std::string_view`,
    ///    and `V` can be TagValue, `std::string`, `double`,
    ///    `int` or `bool`)
    ///
    template<typename T>
    operator std::vector<T>() const // NOLINT(google-explicit-constructor)
    {
        std::vector<T> list;
        // TODO: populate
        return list;
    }
};

} // namespace geodesk