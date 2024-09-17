// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "Feature.h"

namespace geodesk {

///
/// @brief A Feature that represents a single point.
///
class Node : public Feature
{
public:
    /// @name Type & Identity
    /// @{

    /// @brief `FeatureType::NODE`
    ///
    FeatureType type() const noexcept;

    /// @brief The ID of this Node
    ///
    /// @return the ID of the Node
    int64_t id() const noexcept;

    /// @brief Always `true`
    ///
    bool isNode() const noexcept;

    /// @brief `true` if this Feature is an *anonymous node*.
    ///
    /// An anonymous node is a Node that has no tags and does not belong
    /// to any relation; it merely exists as a vertex of a Way.
    /// Its ID is always `0` and its `FeaturePtr` is a `nullptr`.
    ///
    bool isAnonymousNode() const noexcept;

    /// @brief Always `false`
    ///
    bool isWay() const noexcept;

    /// @brief Always `false`
    ///
    bool isRelation() const noexcept;

    /// @brief Always `false`
    ///
    bool isArea() const noexcept;

    /// `true` if this Node belongs to at least one Relation.
    /// Always `false` if this Node is an anonymous node/
    ///
    bool belongsToRelation() const noexcept;

    /// @brief The Node's role in its Relation, if it was returned
    /// via members() or Features::membersOf(), otherwise an empty string.
    ///
    /// @return the Node's role (or an empty string)
    ///
    StringValue role() const noexcept;

    /// @}
};

} // namespace geodesk



