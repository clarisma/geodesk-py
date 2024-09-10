// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "FeaturesBase.h"
#include "Node.h"
#include "Way.h"
#include "Relation.h"

namespace geodesk {

using geodesk::detail::View;

class Nodes : public geodesk::detail::FeaturesBase<Node>
{
public:
	using FeaturesBase::FeaturesBase;
	Nodes(const FeaturesBase<Feature>& other) : 
		FeaturesBase(View(other.view_ & FeatureTypes::NODES)) {}
	Nodes(const FeaturesBase<Node>& other) : FeaturesBase(other.view_) {}
	Nodes(const FeaturesBase<Way>& other) : FeaturesBase(empty()) {}
	Nodes(const FeaturesBase<Relation>& other) : FeaturesBase(empty()) {}

	template<typename Ptr, bool N, bool W, bool R>
	Nodes nodesOf(detail::FeatureBase<Ptr,N,W,R> feature) const;
};

} // namespace geodesk
