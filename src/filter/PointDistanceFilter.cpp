// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "PointDistanceFilter.h"
#include <common/util/log.h>
#include "feature/FeatureStore.h"
#include "feature/FastMemberIterator.h"
#include "feature/WayPtr.h"
#include "feature/polygon/PointInPolygon.h"
#include "geom/Distance.h"

// TODO: Conditionally accelerate if distance is far enough that
// some tiles could lie outside the circle

PointDistanceFilter::PointDistanceFilter(double meters, Coordinate point)
	: point_(point)
{
	double d = Mercator::unitsFromMeters(meters, point.y);
	bounds_ = Box::unitsAroundXY((int32_t)std::ceil(d), point);
	distanceSquared_ = d * d;
}


bool PointDistanceFilter::segmentsWithinDistance(WayPtr way, int areaFlag) const
{
    WayCoordinateIterator iter;
    iter.start(way, areaFlag);
    Coordinate c = iter.next();
    double x1 = c.x;
    double y1 = c.y;
    for(;;)
    {
        c = iter.next();
        if (c.isNull()) break;
        double x2 = c.x;
        double y2 = c.y;
        if (Distance::pointSegmentSquared(x1, y1, x2, y2,
            point_.x, point_.y) < distanceSquared_)
        {
            return true;
        }
        x1 = x2;
        y1 = y2;
    }
    return false;
}


bool PointDistanceFilter::isWithinDistance(WayPtr way) const
{
    if (way.isArea())
    {
        if (segmentsWithinDistance(way, FeatureFlags::AREA)) return true;
        // The distance of a point that lies within a polygon is zero;
        // we need to perform p-in-p check because the edges themselves
        // may be far away from the comparison point
        // TODO: check bbox first?
        Box bounds = way.bounds();
        if (!bounds.contains(point_)) return false;
        PointInPolygon pip(point_);
        pip.testAgainstWay(way);
        return pip.isInside();
        /*
        if (point_.y >= bounds.minY() && point_.y <= bounds.maxY())
        {
            return way.containsPointFast(point_.x, point_.y);
        }
        */
    }
    return segmentsWithinDistance(way, 0);
}


bool PointDistanceFilter::isAreaWithinDistance(FeatureStore* store, RelationPtr relation) const
{
    // measure distance to the ways that define shell and holes, and
    // also perform point in polygon test
    int odd = 0;
    PointInPolygon pip(point_);
    FastMemberIterator iter(store, relation);
    for (;;)
    {
        FeaturePtr member = iter.next();
        if (member.isNull()) break;
        if (!member.isWay()) continue;
        WayPtr memberWay(member);
        if (memberWay.isPlaceholder()) continue;
        int memberFlags = member.flags();
        if (segmentsWithinDistance(memberWay, memberFlags)) return true;
        /*
        Box bounds = memberWay.bounds();
        if (point_.y >= bounds.minY() && point_.y <= bounds.maxY())
        {
            odd ^= memberWay.containsPointFast(point_.x, point_.y);
            LOG("odd = %d", odd);
        }
        */
        // No bbox check needed, testAgainstWay() does it
        pip.testAgainstWay(memberWay);
    }
    // LOG("final odd = %d", odd);
    // return odd != 0;
    return pip.isInside();
}


bool PointDistanceFilter::accept(FeatureStore* store, FeaturePtr feature, FastFilterHint fast) const
{
    int type = feature.typeCode();
    if (type == FeatureType::WAY)
    {
        WayPtr way(feature);
        return isWithinDistance(way);
    }
    if (type == FeatureType::NODE)
    {
        NodePtr node(feature);
        return Distance::pointsSquared(node.x(), node.y(), 
            point_.x, point_.y) < distanceSquared_;
    }
    assert(type == FeatureType::RELATION);
    if (feature.isArea())
    {
        return isAreaWithinDistance(store, RelationPtr(feature));
    }
    RelationPtr relation(feature);
    RecursionGuard guard(relation);
    return areMembersWithinDistance(store, relation, guard);
}


bool PointDistanceFilter::areMembersWithinDistance(FeatureStore* store, RelationPtr relation, RecursionGuard& guard) const
{
    FastMemberIterator iter(store, relation);
    for (;;)
    {
        FeaturePtr member = iter.next();
        if (member.isNull()) break;
        int typeCode = member.typeCode();
        if (typeCode == 1)
        {
            WayPtr memberWay(member);
            if (!memberWay.isPlaceholder() && isWithinDistance(memberWay)) return true;
        }
        else if (typeCode == 0)
        {
            NodePtr memberNode(member);
            if (!memberNode.isPlaceholder() && Distance::pointsSquared(
                memberNode.x(), memberNode.y(), point_.x, point_.y) < distanceSquared_)
            {
                return true;
            }
        }
        else
        {
            assert(member.isRelation());
            RelationPtr memberRel(member);
            if (!memberRel.isPlaceholder() && guard.checkAndAdd(memberRel) &&
                areMembersWithinDistance(store, memberRel, guard))
            {
                return true;
            }
        }
    }
    return false;
}
