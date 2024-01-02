// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "MemberCollection.h"
#include "FastMemberIterator.h"
#include "Node.h"
#include "Way.h"

MemberCollection::MemberCollection(FeatureStore* store, RelationRef relation) :
	types_(0)
{
	RecursionGuard guard(relation);
	collect(store, relation, guard);
}

void MemberCollection::collect(FeatureStore* store, RelationRef relation, RecursionGuard& guard)
{
	FastMemberIterator iter(store, relation);
	for (;;)
	{
		FeatureRef member = iter.next();
		if (member.isNull()) break;
		int memberType = member.typeCode();
		if (memberType == 1)
		{
			WayRef memberWay(member);
			if (memberWay.isPlaceholder()) continue;
			types_ |= LINEAL;
		}
		else if (memberType == 0)
		{
			NodeRef memberNode(member);
			if (memberNode.isPlaceholder()) continue;
			types_ |= PUNTAL;
		}
		else
		{
			assert(memberType == 2);
			RelationRef childRel(member);
			if (childRel.isPlaceholder() || !guard.checkAndAdd(childRel)) continue;
			if (childRel.isArea())
			{
				types_ |= POLYGONAL;
			}
			else
			{
				collect(store, childRel, guard);
				continue;	// Don't add the relation itself
			}
		}
		push_back(member);
	}
}
