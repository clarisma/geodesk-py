// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "Feature.h"

namespace geodesk {

///
/// @brief A @ref Feature that represents a grouping of related features
/// or a complex polygon. Each Relation has one or more member features,
/// which can be retrieved via members(). Each Feature object returned
/// by members() has its role set to a non-empty string, if it has
/// an explicit role in this Relation (e.g. `"stop"` for a Node in a
/// route relation that represents a bus stop).
///
/// **Note:** Functions that require the complete geometry of a Relation
/// (such as area() or centroid()) may fails with a QueryException if
/// one or more tiles that contain members of the Relation are missing.
///
class Relation : public Feature
{
public:
    /// @name Type & Identity
    /// @{

    /// @brief `FeatureType::RELATION`
    ///
    FeatureType type() const noexcept;

    /// @brief The ID of this Relation
    ///
    int64_t id() const noexcept;

    /// @brief Always `false`
    ///
    bool isNode() const noexcept;

    /// @brief Always `false`
    ///
    bool isAnonymousNode() const noexcept;

    /// @brief Always `false`
    ///
    bool isWay() const noexcept;

    /// @brief Always `true`
    ///
    bool isRelation() const noexcept;

    /// @brief `true` if this Relation represents an area.
    ///
    bool isArea() const noexcept;

    /// `true` if this Relation itself belongs to at least one Relation.
    ///
    bool belongsToRelation() const noexcept;

    /// @brief The Relation's role in its parent Relation, if it was returned
    /// via members() or Features::membersOf(), otherwise an empty string.
    ///
    /// @return the Relation's role (or an empty string)
    ///
    StringValue role() const noexcept;

    /// @}
};

} // namespace geodesk



