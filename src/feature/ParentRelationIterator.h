// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "Relation.h"

class ParentRelationIterator
{
public:
	ParentRelationIterator(FeatureStore* store, pointer pRelTable,
		const MatcherHolder* matcher, const Filter* filter);

	FeatureStore* store() const { return store_; }
	RelationRef next();

private:
	FeatureStore* store_;
	const MatcherHolder* matcher_;
	const Filter* filter_;
	Tip currentTip_;
	int32_t currentRel_;
	pointer p_;
	pointer pForeignTile_;
};