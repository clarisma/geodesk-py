// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "Length.h"
#include "feature/FastMemberIterator.h"
#include "geom/Mercator.h"

double Length::ofWay(WayRef way)
{
    double d = 0;
    WayCoordinateIterator iter(way);
    Coordinate p1 = iter.next();
    for (;;)
    {
        Coordinate p2 = iter.next();
        if (p2.isNull()) break;
        d += Mercator::distance(p1, p2);
        p1 = p2;
    }
    return d;
}

// TODO: Define in spec: what's the "length" of an Area-Relation?
// Circumference without holes? Right now, we simply add up all the ways

double Length::ofRelation(FeatureStore* store, RelationRef relation)
{
	RecursionGuard guard(relation);
	return ofRelation(store, relation, guard);
}

double Length::ofRelation(FeatureStore *store, RelationRef rel, RecursionGuard &guard)
{
	double totalLength = 0;
	FastMemberIterator iter(store, rel);
	for (;;)
	{
		FeatureRef member = iter.next();
		if (member.isNull()) break;
		int memberType = member.typeCode();
		if (memberType == 1)
		{
			totalLength += ofWay(WayRef(member));		// This is placeholder-safe
		}
		else if (memberType == 2)
		{
			RelationRef childRel(member);
			if (guard.checkAndAdd(childRel))
			{
				totalLength += ofRelation(store, childRel, guard);		// This is placeholder-safe
			}
		}
	}
	return totalLength;
}

