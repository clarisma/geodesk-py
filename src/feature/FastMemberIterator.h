// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "RelationPtr.h"

class FastMemberIterator
{
public:
	FastMemberIterator(FeatureStore* store, RelationPtr relation);

	FeatureStore* store() const { return store_; }
	FeaturePtr next();

private:
	FeatureStore* store_;
	Tip currentTip_;
	int32_t currentMember_;
	DataPtr p_;
	DataPtr pForeignTile_;
};

