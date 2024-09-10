// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "SpatialFilter.h"
#include "feature/FastMemberIterator.h"

bool SpatialFilter::acceptFeature(FeatureStore* store, FeaturePtr feature) const
{
	int type = feature.typeCode();
	if (type == 1) return acceptWay(WayPtr(feature));
	if (type == 0) return acceptNode(NodePtr(feature));
	
	assert(feature.isRelation());
	RelationPtr relation(feature);
	if (relation.isArea()) return acceptAreaRelation(store, relation);
	RecursionGuard guard(relation);
	return acceptMembers(store, relation, guard);
}

// TODO: check member bboxes as a quick way to eliminate
bool SpatialFilter::acceptMembers(FeatureStore* store, RelationPtr relation, RecursionGuard& guard) const
{
	FastMemberIterator iter(store, relation);
	for (;;)
	{
		FeaturePtr member = iter.next();
		if (member.isNull()) break;
		bool memberAccepted;
		int memberType = member.typeCode();
		if (memberType == 1)
		{
			WayPtr memberWay(member);
			if (memberWay.isPlaceholder()) continue;
			memberAccepted = acceptWay(memberWay);
		}
		else if (memberType == 0)
		{
			NodePtr memberNode(member);
			if (memberNode.isPlaceholder()) continue;
			memberAccepted = acceptNode(memberNode);
		}
		else
		{
			assert(memberType == 2);
			RelationPtr childRel(member);
			if (childRel.isPlaceholder() || !guard.checkAndAdd(childRel)) continue;
			if (childRel.isArea())
			{
				memberAccepted = acceptAreaRelation(store, childRel);
			}
			else
			{
				memberAccepted = acceptMembers(store, childRel, guard);
			}
		}
		if (flags_ & FilterFlags::MUST_ACCEPT_ALL_MEMBERS)
		{
			if (!memberAccepted) return false;
		}
		else
		{
			if (memberAccepted) return true;
		}
	}
	return (flags_ & FilterFlags::MUST_ACCEPT_ALL_MEMBERS) != 0;
}
