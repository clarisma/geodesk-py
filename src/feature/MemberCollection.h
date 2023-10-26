// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <vector>
#include "Relation.h"

class MemberCollection : public std::vector<FeatureRef>
{
public:
	MemberCollection(FeatureStore* store, RelationRef relation);

	enum
	{
		PUNTAL = 1,
		LINEAL = 2,
		POLYGONAL = 4,
	};

private:
	void collect(FeatureStore* store, RelationRef relation, RecursionGuard& guard);

	int types_;
};
