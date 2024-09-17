// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "FeaturesBase.h"
#include "Node.h"
#include "Way.h"
#include "Relation.h"

namespace geodesk {

using geodesk::detail::View;

class Relations : public geodesk::detail::FeaturesBase<Relation>
{
public:
	using FeaturesBase::FeaturesBase;
	Relations(const FeaturesBase<Feature>& other) :
		FeaturesBase(View(other.view_& FeatureTypes::RELATIONS)) {}
	Relations(const FeaturesBase<Node>& other) : FeaturesBase(empty()) {}
	Relations(const FeaturesBase<Way>& other) : FeaturesBase(empty()) {}
	Relations(const FeaturesBase<Relation>& other) : FeaturesBase(other.view_) {}

	template<typename Ptr, bool N, bool W, bool R>
	Relations parentsOf(detail::FeatureBase<Ptr,N,W,R> feature) const
	{
		if(!feature.belongsToRelation()) return Relations(empty());
		return Relations(view_.parentRelationsOf(feature.ptr()));
	}

	template<typename Ptr, bool N, bool W, bool R>
	Relations membersOf(detail::FeatureBase<Ptr,N,W,R> feature) const
	{
		// Relations can only be members of other relations
		if(!feature.isRelation()) return Relations(empty());
		return Relations(view_.membersOfRelation(feature.ptr()));
	}

};

} // namespace geodesk
