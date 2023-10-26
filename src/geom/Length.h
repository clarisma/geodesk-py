// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "feature/Way.h"
#include "feature/Relation.h"

class Length
{
public:
	static double ofWay(WayRef way);
	static double ofRelation(FeatureStore* store, RelationRef relation);

private:
	static double ofRelation(FeatureStore* store, RelationRef rel, RecursionGuard& guard);
};
