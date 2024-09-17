// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#ifndef GEODESK_DOXYGEN

#include "FeaturesBase.h"
#include "Nodes.h"
#include "Ways.h"
#include "Relations.h"

namespace geodesk {

class Features : public detail::FeaturesBase<Feature>
{
public:
	using FeaturesBase::FeaturesBase;
	Features(const char* golFile) :
		FeaturesBase(rootView(golFile))
	{
	}

	template <typename T>
	Features(const FeaturesBase<T>& other) :
		FeaturesBase(other.view_)
	{
	}

	///
	/// Returns a view of this collection that contains only nodes.
	///
	Nodes nodes() const { return Nodes(view_ & FeatureTypes::NODES); };
	
	///
	/// Returns a view of this collection that contains only nodes 
	/// matching the given query.
	/// 
	/// @param query a query in <a href="https://docs.geodesk.com/goql">GOQL</a> format 
	/// 
	Nodes nodes(const char* query) const
	{
		return Nodes(view_.withQuery(query, FeatureTypes::NODES));
	}
	Ways ways() const {	return Ways(view_ & FeatureTypes::WAYS); }
	Ways ways(const char* query) const
	{
		return Ways(view_.withQuery(query, FeatureTypes::WAYS));
	}
	Relations relations() const { return Relations(view_ & FeatureTypes::RELATIONS); }
	Relations relations(const char* query) const
	{
		return Relations(view_.withQuery(query, FeatureTypes::RELATIONS));
	}

	template<typename Ptr, bool N, bool W, bool R>
	Nodes nodesOf(detail::FeatureBase<Ptr,N,W,R> feature) const;

	template<typename Ptr, bool N, bool W, bool R>
	FeaturesBase parentsOf(detail::FeatureBase<Ptr,N,W,R> feature) const
	{
		if(view_.types() & (FeatureTypes::WAYS | FeatureTypes::RELATIONS))
		{
			// TODO
		}
		return FeaturesBase(empty());
	}

};

} // namespace geodesk

#endif