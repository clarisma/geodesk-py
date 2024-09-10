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


void WktWriter::writeAnonymousNodeNode(Coordinate point)
{
	if (!firstFeature_) writeConstString(", ");
	writeConstString("POINT(");
	writeCoordinate(point);
	writeByte(')');
	firstFeature_ = false;
}


void WktWriter::writeNodeGeometry(NodePtr node)
{
	writeConstString("POINT(");
	writeCoordinate(node.xy());
	writeByte(')');
}

void WktWriter::writeWayGeometry(WayPtr way)
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

void WktWriter::writeAreaRelationGeometry(FeatureStore* store, RelationPtr relation)
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


void WktWriter::writeCollectionRelationGeometry(FeatureStore* store, RelationPtr relation)
{
	writeConstString("GEOMETRYCOLLECTION");
	if (writeMemberGeometries(store, relation) == 0) writeConstString(" EMPTY");
}



void WktWriter::writeFeature(FeatureStore* store, FeaturePtr feature)
{
	if (!firstFeature_) writeConstString(", ");
	writeFeatureGeometry(store, feature); 
	firstFeature_ = false;
}


void WktWriter::writeHeader()
{
	writeConstString("GEOMETRYCOLLECTION(");
}

void WktWriter::writeFooter()
{
	writeConstString(")");
}

