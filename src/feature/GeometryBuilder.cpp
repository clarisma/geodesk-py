// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "GeometryBuilder.h"

#include "feature/polygon/Polygonizer.h"
#include "feature/polygon/Ring.h"

GEOSGeometry* GeometryBuilder::buildWayGeometry(const FeaturePtr way, GEOSContextHandle_t geosContext)
{
	WayCoordinateIterator iter;
	int areaFlag = way.flags() & FeatureFlags::AREA;
	iter.start(way, areaFlag);
	int count = iter.storedCoordinatesRemaining() + (areaFlag ? 1 : 0);
	GEOSCoordSequence* coordSeq = GEOSCoordSeq_create_r(geosContext, count, 2);  // 2D points
	for (int i=0; i<count; i++)
	{
		Coordinate c = iter.next();
		GEOSCoordSeq_setXY_r(geosContext, coordSeq, i, c.x, c.y);
	}
	if (areaFlag)
	{
		GEOSGeometry* exteriorRing = GEOSGeom_createLinearRing_r(geosContext, coordSeq);
		return GEOSGeom_createPolygon_r(geosContext, exteriorRing, NULL, 0);
	}
	else
	{
		return GEOSGeom_createLineString_r(geosContext, coordSeq);
	}
}

// TODO: consolidate with buildPointGeometry
GEOSGeometry* GeometryBuilder::buildNodeGeometry(const NodePtr node, GEOSContextHandle_t geosContext)
{
	GEOSCoordSequence* coordSeq = GEOSCoordSeq_create_r(geosContext, 1, 2);  // 1 point, 2D (X and Y)
	GEOSCoordSeq_setXY_r(geosContext, coordSeq, 0, node.x(), node.y());
	return GEOSGeom_createPoint_r(geosContext, coordSeq);
}

GEOSGeometry* GeometryBuilder::buildPointGeometry(int32_t x, int32_t y, GEOSContextHandle_t geosContext)
{
	GEOSCoordSequence* coordSeq = GEOSCoordSeq_create_r(geosContext, 1, 2);  // 1 point, 2D (X and Y)
	GEOSCoordSeq_setXY_r(geosContext, coordSeq, 0, x, y);
	return GEOSGeom_createPoint_r(geosContext, coordSeq);
}


GEOSGeometry* GeometryBuilder::buildBoxGeometry(const Box& box, GEOSContextHandle_t geosContext)
{
	GEOSCoordSequence* coordSeq = GEOSCoordSeq_create_r(geosContext, 5, 2);  // 5 points, 2D
	GEOSCoordSeq_setXY_r(geosContext, coordSeq, 0, box.minX(), box.minY());
	GEOSCoordSeq_setXY_r(geosContext, coordSeq, 1, box.minX(), box.maxY());
	GEOSCoordSeq_setXY_r(geosContext, coordSeq, 2, box.maxX(), box.maxY());
	GEOSCoordSeq_setXY_r(geosContext, coordSeq, 3, box.maxX(), box.minY());
	GEOSCoordSeq_setXY_r(geosContext, coordSeq, 4, box.minX(), box.minY());
	GEOSGeometry* exteriorRing = GEOSGeom_createLinearRing_r(geosContext, coordSeq);
	return GEOSGeom_createPolygon_r(geosContext, exteriorRing, NULL, 0);
}


GEOSGeometry* GeometryBuilder::buildAreaRelationGeometry(FeatureStore* store, RelationPtr relation, GEOSContextHandle_t geosContext)
{
	Polygonizer polygonizer;
	polygonizer.createRings(store, relation);
	polygonizer.assignAndMergeHoles();
	return polygonizer.createPolygonal(geosContext);
}


GEOSGeometry* GeometryBuilder::buildRelationGeometry(FeatureStore* store, RelationPtr relation, GEOSContextHandle_t geosContext)
{
	if (relation.isArea())
	{
		return buildAreaRelationGeometry(store, relation, geosContext);
	}
	RelationGeometryBuilder rgb(store, relation, geosContext);
	return rgb.build();
}


RelationGeometryBuilder::RelationGeometryBuilder(FeatureStore* store, 
	RelationPtr relation, GEOSContextHandle_t geosContext) :
	store_(store),
	context_(geosContext),
	guard_(relation)
{
	gatherMembers(relation);
}


void RelationGeometryBuilder::gatherMembers(RelationPtr relation)
{
	FastMemberIterator iter(store_, relation);
	for (;;)
	{
		FeaturePtr member = iter.next();
		if (member.isNull()) break;
		int memberType = member.typeCode();
		GEOSGeometry* g;
		if (memberType == 1)
		{
			WayPtr memberWay(member);
			if (memberWay.isPlaceholder()) continue;
			g = GeometryBuilder::buildWayGeometry(memberWay, context_);
		}
		else if (memberType == 0)
		{
			NodePtr memberNode(member);
			if (memberNode.isPlaceholder()) continue;
			g = GeometryBuilder::buildNodeGeometry(memberNode, context_);
		}
		else
		{
			assert(memberType == 2);
			RelationPtr childRel(member);
			if (childRel.isPlaceholder() || !guard_.checkAndAdd(childRel)) continue;
			if (childRel.isArea())
			{
				g = GeometryBuilder::buildAreaRelationGeometry(store_, childRel, context_);
			}
			else
			{
				gatherMembers(childRel);
				continue;
			}
		}
		geoms_.push_back(g);
	}
}


GEOSGeometry* RelationGeometryBuilder::build()
{
	// TODO: different collection types

	return GEOSGeom_createCollection_r(context_, GEOS_GEOMETRYCOLLECTION, 
		geoms_.data(), static_cast<unsigned int>(geoms_.size()));
		// TODO: Update this if GEOSGeom_createCollection_r is ever
		// updated to use size_t for ngeoms 
}


GEOSGeometry* GeometryBuilder::buildFeatureGeometry(FeatureStore* store, FeaturePtr feature, GEOSContextHandle_t geosContext)
{
	int typeCode = feature.typeCode();
	if (typeCode == 1)
	{
		return buildWayGeometry(WayPtr(feature), geosContext);
	}
	if (typeCode == 0)
	{
		return buildNodeGeometry(NodePtr(feature), geosContext);
	}
	assert(feature.isRelation());
	return buildRelationGeometry(store, RelationPtr(feature), geosContext);
}
