// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <utility>
#include "TagValue.h"

namespace geodesk {

/// @brief A key/value pair that describes an attribute of a Feature.
///
/// Converts implicitly to `std::pair<K,V>`.
///
/// **Warning**: A Tag object is a lightweight wrapper
/// around a pointer to a Feature's tag data, contained
/// in a FeatureStore. It becomes invalid once that
/// FeatureStore is closed. To use a Feature's keys and
/// values beyond the lifetime of the FeatureStore,
/// convert them to `std::string`, which allocates
/// copies.
///
/// @see Tags, TagValue
///
class Tag
{
public:
    /// @brief Returns this tag as a `std::pair`
    ///
    /// @param K  StringValue, `std::string` or `std::string_view`
    /// @param V  TagValue, `std::string`, `double`, `int` or `bool`
    ///
    template<typename K, typename V>
    operator std::pair<K,V>() const noexcept
    {
        return {key_,value_};
    }

private:
    StringValue key_;
    TagValue value_;
};

} // geodesk
