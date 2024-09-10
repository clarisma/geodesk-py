// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "SpatialFilter.h"

class ConnectedFilter : public SpatialFilter
{
public:
	ConnectedFilter(FeatureStore* store, FeaturePtr feature);

	bool accept(FeatureStore* store, FeaturePtr feature, FastFilterHint fast) const override;

protected:
	bool acceptWay(WayPtr way) const override;
	bool acceptNode(NodePtr node) const override;
	bool acceptAreaRelation(FeatureStore* store, RelationPtr relation) const override;

private:
	void collectWayPoints(WayPtr way);
	void collectMemberPoints(FeatureStore* store, RelationPtr relation, RecursionGuard& guard);

	uint64_t self_;
	std::unordered_set<Coordinate> points_;
};
