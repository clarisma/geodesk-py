// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#ifndef GEODESK_DOXYGEN

#include "FeatureBase.h"
#include "feature/WayPtr.h"

namespace geodesk {
///
/// @brief A Feature that represents a linestring, linear ring,
/// or a simple polygon.
///
/// Every Way has two or more nodes, which
/// can be retrieved via @ref nodes.
///
class Way : public detail::FeatureBase<WayPtr, false, true, false> // -W-
{
public:
    using FeatureBase::FeatureBase;
};

} // namespace geodesk

#endif