// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "feature/WayPtr.h"
#include "feature/RelationPtr.h"

class Length
{
public:
	static double ofWay(WayPtr way);
	static double ofRelation(FeatureStore* store, RelationPtr relation);

private:
	static double ofRelation(FeatureStore* store, RelationPtr rel, RecursionGuard& guard);
};
