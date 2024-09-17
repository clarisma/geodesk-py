// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "feature/MemberCollection.h"
#include "feature/FastMemberIterator.h"
#include "feature/NodePtr.h"
#include "feature/WayPtr.h"

MemberCollection::MemberCollection(FeatureStore* store, RelationPtr relation) :
	types_(0)
{
	RecursionGuard guard(relation);
	collect(store, relation, guard);
}

void MemberCollection::collect(FeatureStore* store, RelationPtr relation, RecursionGuard& guard)
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
			types_ |= LINEAL;
		}
		else if (memberType == 0)
		{
			NodePtr memberNode(member);
			if (memberNode.isPlaceholder()) continue;
			types_ |= PUNTAL;
		}
		else
		{
			assert(memberType == 2);
			RelationPtr childRel(member);
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
