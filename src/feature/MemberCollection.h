// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <vector>
#include "RelationPtr.h"

class MemberCollection : public std::vector<FeaturePtr>
{
public:
	MemberCollection(FeatureStore* store, RelationPtr relation);

	enum
	{
		PUNTAL = 1,
		LINEAL = 2,
		POLYGONAL = 4,
	};

private:
	void collect(FeatureStore* store, RelationPtr relation, RecursionGuard& guard);

	int types_;
};
