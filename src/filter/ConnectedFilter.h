// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "SpatialFilter.h"

class ConnectedFilter : public SpatialFilter
{
public:
	ConnectedFilter(FeatureStore* store, FeatureRef feature);

	bool accept(FeatureStore* store, FeatureRef feature, FastFilterHint fast) const override;

protected:
	bool acceptWay(WayRef way) const override;
	bool acceptNode(NodeRef node) const override;
	bool acceptAreaRelation(FeatureStore* store, RelationRef relation) const override;

private:
	void collectWayPoints(WayRef way);
	void collectMemberPoints(FeatureStore* store, RelationRef relation, RecursionGuard& guard);

	uint64_t self_;
	std::unordered_set<Coordinate> points_;
};
