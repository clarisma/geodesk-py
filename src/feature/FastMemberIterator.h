// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "Relation.h"

class FastMemberIterator
{
public:
	FastMemberIterator(FeatureStore* store, const RelationRef relation);

	FeatureStore* store() const { return store_; }
	FeatureRef next();

private:
	FeatureStore* store_;
	Tip currentTip_;
	int32_t currentMember_;
	pointer p_;
	pointer pForeignTile_;
};

