// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "Filter.h"
#include "feature/Way.h"
#include "geom/Mercator.h"
#include "match/Matcher.h"



class WithMembersFilter : public Filter
{
public:
	WithMembersFilter(FeatureTypes types, const MatcherHolder* memberMatcher, int minCount, int maxCount) :
		Filter(0, types),
		memberMatcher_(memberMatcher),
		minCount_(minCount),
		maxCount_(maxCount)
	{
		memberMatcher->addref();
	}
	~WithMembersFilter()
	{
		memberMatcher_->release();
	}

private:
	const MatcherHolder* memberMatcher_;
	int minCount_;
	int maxCount_;
};



