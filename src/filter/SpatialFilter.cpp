// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "SpatialFilter.h"
#include "feature/FastMemberIterator.h"

bool SpatialFilter::acceptFeature(FeatureStore* store, FeatureRef feature) const
{
	int type = feature.typeCode();
	if (type == 1) return acceptWay(WayRef(feature));
	if (type == 0) return acceptNode(NodeRef(feature));
	
	assert(feature.isRelation());
	RelationRef relation(feature);
	if (relation.isArea()) return acceptAreaRelation(store, relation);
	RecursionGuard guard(relation);
	return acceptMembers(store, relation, guard);
}

// TODO: check member bboxes as a qucik way to eliminate
bool SpatialFilter::acceptMembers(FeatureStore* store, RelationRef relation, RecursionGuard& guard) const
{
	FastMemberIterator iter(store, relation);
	for (;;)
	{
		FeatureRef member = iter.next();
		if (member.isNull()) break;
		bool memberAccepted;
		int memberType = member.typeCode();
		if (memberType == 1)
		{
			WayRef memberWay(member);
			if (memberWay.isPlaceholder()) continue;
			memberAccepted = acceptWay(memberWay);
		}
		else if (memberType == 0)
		{
			NodeRef memberNode(member);
			if (memberNode.isPlaceholder()) continue;
			memberAccepted = acceptNode(memberNode);
		}
		else
		{
			assert(memberType == 2);
			RelationRef childRel(member);
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
