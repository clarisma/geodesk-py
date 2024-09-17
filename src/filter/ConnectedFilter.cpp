// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "ConnectedFilter.h"
#include "feature/FastMemberIterator.h"
#include "feature/WayCoordinateIterator.h"

ConnectedFilter::ConnectedFilter(FeatureStore* store, FeaturePtr feature)
{
	self_ = feature.idBits();
	if (feature.isWay())
	{
		WayPtr way(feature);
		if (!way.isPlaceholder())
		{
			collectWayPoints(way);
			bounds_ = way.bounds();
		}
	}
	else if (feature.isNode())
	{
		NodePtr node(feature);
		if (!node.isPlaceholder())
		{
			Coordinate c = node.xy();
			points_.insert(c);
			bounds_ = Box(c);
		}
	}
	else
	{
		assert(feature.isRelation());
		RelationPtr relation(feature);
		RecursionGuard guard(relation);
		collectMemberPoints(store, relation, guard);
		bounds_ = relation.bounds();
	}
}


void ConnectedFilter::collectWayPoints(WayPtr way)
{
	WayCoordinateIterator iter;
	iter.start(way, 0);
	for (;;)
	{
		Coordinate c = iter.next();
		if (c.isNull()) break;
		points_.insert(c);
	}
}


void ConnectedFilter::collectMemberPoints(FeatureStore* store, RelationPtr relation, RecursionGuard& guard)
{
	FastMemberIterator iter(store, relation);
	for (;;)
	{
		FeaturePtr member = iter.next();
		if (member.isNull()) break;
		int memberType = member.typeCode();
		if (memberType == 1)
		{
			WayPtr memberWay(member);
			if (memberWay.isPlaceholder()) continue;
			collectWayPoints(memberWay);
		}
		else if (memberType == 0)
		{
			NodePtr memberNode(member);
			if (memberNode.isPlaceholder()) continue;
			points_.insert(memberNode.xy());
		}
		else
		{
			RelationPtr childRel(member);
			if (childRel.isPlaceholder() || !guard.checkAndAdd(childRel)) continue;
			collectMemberPoints(store, childRel, guard);
		}
	}
}


bool ConnectedFilter::acceptWay(WayPtr way) const
{
	WayCoordinateIterator iter;
	iter.start(way, 0);
	for (;;)
	{
		Coordinate c = iter.next();
		if (c.isNull()) break;
		if (points_.find(c) != points_.end()) return true;
	}
	return false;
}

bool ConnectedFilter::acceptNode(NodePtr node) const
{
	return points_.find(node.xy()) != points_.end();
}

bool ConnectedFilter::acceptAreaRelation(FeatureStore* store, RelationPtr relation) const
{
	RecursionGuard guard(relation);
	return acceptMembers(store, relation, guard);
}


bool ConnectedFilter::accept(FeatureStore* store, FeaturePtr feature, FastFilterHint fast) const
{
	if (feature.idBits() == self_) return false;
	return acceptFeature(store, feature);
}

