// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "WktWriter.h"
#include "feature/FeatureStore.h"
#include "feature/polygon/Polygonizer.h"
#include "feature/polygon/Ring.h"

WktWriter::WktWriter(Buffer* buf) : FeatureWriter(buf)
{
	coordValueSeparatorChar_ = ' ';
	coordStartChar_ = 0;
	coordEndChar_ = 0;
	coordGroupStartChar_ = '(';
	coordGroupEndChar_ = ')';
}


void WktWriter::writeNodeGeometry(NodeRef node)
{
	writeConstString("POINT(");
	writeCoordinate(node.xy());
	writeByte(')');
}

void WktWriter::writeWayGeometry(WayRef way)
{
	if (way.isArea())
	{
		writeConstString("POLYGON");
	}
	else
	{
		writeConstString("LINESTRING");
	}
	writeWayCoordinates(way, way.isArea());
}

void WktWriter::writeAreaRelationGeometry(FeatureStore* store, RelationRef relation)
{
	Polygonizer polygonizer;
	polygonizer.createRings(store, relation);
	polygonizer.assignAndMergeHoles();
	const Polygonizer::Ring* ring = polygonizer.outerRings();
	int count = ring ? (ring->next() ? 2 : 1) : 0;
	if (count > 1)
	{
		writeConstString("MULTIPOLYGON");
	}
	else
	{
		writeConstString("POLYGON");
	}
	if (count == 0)
	{
		writeConstString(" EMPTY");
	}
	else
	{
		writePolygonizedCoordinates(polygonizer);
	}
}


void WktWriter::writeMemberGeometries(FeatureStore* store, RelationRef relation, RecursionGuard& guard)
{
	// TODO
}


void WktWriter::writeFeature(FeatureStore* store, FeatureRef feature)
{
	if (!firstFeature_)
	{
		writeConstString(", ");
	}
	if (feature.isWay())
	{
		writeWayGeometry(WayRef(feature));
	}
	else if (feature.isNode())
	{
		writeNodeGeometry(NodeRef(feature));
	}
	else
	{
		assert(feature.isRelation());
		RelationRef relation(feature);
		if (relation.isArea())
		{
			writeAreaRelationGeometry(store, relation);
		}
		else
		{
			RecursionGuard guard(relation);
			writeConstString("GEOMETRYCOLLECTION(");
			writeMemberGeometries(store, relation, guard);
			writeByte(')');
		}
	}
}


void WktWriter::writeHeader()
{
	writeConstString("GEOMETRYCOLLECTION(");
}

void WktWriter::writeFooter()
{
	writeConstString(")");
}

