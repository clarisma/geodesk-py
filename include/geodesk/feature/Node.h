// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#ifndef GEODESK_DOXYGEN
// Doxygen should only see the "fake" class in api

#include "FeatureBase.h"
#include <common/util/TaggedPtr.h>
#include "feature/NodePtr.h"
#include "feature/WayPtr.h"
#include "feature/RelationPtr.h"
#include "feature/FeatureStore.h"
#include "geom/Coordinate.h"


namespace geodesk {

///
/// @brief A Feature that represents a single point.
///
class Node : public detail::FeatureBase<NodePtr, true, false, false> // N--
{
public:
    using FeatureBase::FeatureBase;
};

/*
 * @class NodeAlias
 *
 * @brief A Node, a glorious Node!
 */
using NodeAlias = Node;


} // namespace geodesk

#endif