// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "Feature.h"

namespace geodesk {

///
/// @brief A Feature that represents a linestring, linear ring,
/// or a simple polygon.
///
/// Every Way has two or more nodes, which can be retrieved via nodes().
///
class Way : public Feature
{
public:
    /// @name Type & Identity
    /// @{

    /// @brief `FeatureType::WAY`
    ///
    FeatureType type() const noexcept;

    /// @brief The ID of this Way
    ///
    int64_t id() const noexcept;

    /// @brief Always `false`
    ///
    bool isNode() const noexcept;

    /// @brief Always `false`
    ///
    bool isAnonymousNode() const noexcept;

    /// @brief Always `true`
    ///
    bool isWay() const noexcept;

    /// @brief Always `false`
    ///
    bool isRelation() const noexcept;

    /// @brief `true` if this Way represents an area.
    ///
    bool isArea() const noexcept;

    /// `true` if this Way belongs to at least one Relation.
    ///
    bool belongsToRelation() const noexcept;

    /// @brief The Way's role in its Relation, if it was returned
    /// via members() or Features::membersOf(), otherwise an empty string.
    ///
    /// @return the Way's role (or an empty string)
    ///
    StringValue role() const noexcept;

    /// @}
};

} // namespace geodesk



