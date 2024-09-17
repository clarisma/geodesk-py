// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <map>
#include <unordered_map>
#include <vector>
#include "Tag.h"

namespace geodesk {

/// @brief An object describing the key/value attributes
/// of a Feature.
/// Tags are obtained via Feature::tags() and can be
/// iterated or implicitly converted to a `std::vector`,
/// `std::map` or `std::unordered_map`.
///
/// ```
/// Tags tags = hotel.tags();
/// for(Tag tag: tags)
/// {
///     std::cout << tag.key() << " = " << tag.value() << std::endl;
/// }
/// ```
///
/// **Note:** By default, tags are returned in *storage order*
/// of their keys, which is the same for all tag sets stored in
/// the same GOL, but which generally is not *alphabetical order*.
/// If you want tags sorted alphabetically by key, assign the
/// Tags object to a `std::map`, or to a `std::vector` which you
/// can then explicitly `std::sort()`.
///
/// **Warning**: A Tags object is a lightweight wrapper
/// around a pointer to a Feature's tag data, contained
/// in a FeatureStore. It becomes invalid once that
/// FeatureStore is closed. To use a Feature's keys and
/// values beyond the lifetime of the FeatureStore,
/// assign the Tags object to a `std::vector` or
/// `std::unordered_map` whose types are `std::string`,
/// which allocate copies of the key/value data.
///
/// @see Tag, TagValue
///
class Tags
{
public:
    // TODO
    class Iterator
    {
    public:
        Iterator(const Tags& tags);

        Tag operator*() const;
        Iterator& operator++()
        {
            // TODO
            return *this;
        }

        bool operator!=(std::nullptr_t) const
        {
            return false; // TODO
        }
    };

    /// @brief Returns the number of tags.
    ///
    [[nodiscard]] size_t size() const noexcept;

    /// @brief Looks up the tag value for the given key.
    ///
    /// @return the tag's value (or an empty string
    ///         if no tag with this key exists)
    TagValue operator[](std::string_view key) const noexcept;

    /// @brief Checks if this set of tags contains
    /// a tag with the given key.
    ///
    [[nodiscard]] bool hasTag(std::string_view k) const noexcept;

    /// @brief Checks if this set of tags contains
    /// a tag with the given key and value.
    ///
    [[nodiscard]] bool hasTag(std::string_view k, std::string_view v) const noexcept;

    /// @brief Creates a map of keys to values. Tags are sorted
    /// in alphabetical order of their keys.
    ///
    /// @param K  The key's type: StringValue, `std::string` or `std::string_view`
    /// @param V  The value's type: TagValue, `std::string`, `double`, `int` or `bool`
    ///
    template<typename K, typename V>
    operator std::map<K,V>() const // NOLINT(google-explicit-constructor)
    {
        std::unordered_map<K,V> map;
        // TODO: populate
        return map;
    }

    /// @brief Creates an unordered map of keys to values.
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

    /// Checks if both Tags objects contain the same tags.
    ///
    bool operator==(const Tags& other) const;
    bool operator!=(const Tags& other) const
    {
        return !(*this == other);
    }
    // may allocate, hence throw
};

} // namespace geodesk