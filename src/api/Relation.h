// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "FeatureBase.h"
#include "feature/RelationPtr.h"

namespace geodesk {
///
/// @brief A @ref Feature that represents a grouping of related features
/// or a complex polygon.
///
class Relation : public geodesk::detail::FeatureBase<RelationPtr, false, false, true> // --R
{
public:
    using FeatureBase::FeatureBase;
};

} // namespace geodesk
