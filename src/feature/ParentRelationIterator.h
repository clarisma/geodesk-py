// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "RelationPtr.h"

class ParentRelationIterator
{
public:
	ParentRelationIterator(FeatureStore* store, DataPtr pRelTable,
		const MatcherHolder* matcher, const Filter* filter);

	FeatureStore* store() const { return store_; }
	RelationPtr next();

private:
	FeatureStore* store_;
	const MatcherHolder* matcher_;
	const Filter* filter_;
	Tip currentTip_;
	int32_t currentRel_;
	DataPtr p_;
	DataPtr pForeignTile_;
};