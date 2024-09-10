// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "FeaturesBase.h"
#include "Node.h"
#include "Way.h"
#include "Relation.h"

namespace geodesk {

using geodesk::detail::View;

class Ways : public geodesk::detail::FeaturesBase<Way>
{
public:
	using FeaturesBase::FeaturesBase;
	Ways(const FeaturesBase<Feature>& other) :
		FeaturesBase(View(other.view_& FeatureTypes::WAYS)) {}
	Ways(const FeaturesBase<Node>& other) : FeaturesBase(empty()) {}
	Ways(const FeaturesBase<Way>& other) : FeaturesBase(other.view_) {}
	Ways(const FeaturesBase<Relation>& other) : FeaturesBase(empty()) {}

	template<typename Ptr, bool N, bool W, bool R>
	Ways parentsOf(detail::FeatureBase<Ptr,N,W,R> feature) const;
};

} // namespace geodesk
