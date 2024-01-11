// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "GeoJsonWriter.h"
#include "geodesk.h"
#include "feature/FeatureStore.h"
#include "feature/polygon/Polygonizer.h"
#include "feature/polygon/Ring.h"

void GeoJsonWriter::writeTags(TagIterator& iter)
{
	if (pretty_)
	{
		writeConstString("{\n\t\t\t\t");
	}
	else
	{
		writeByte('{');
	}
	bool first = true;
	std::string_view key;
	TagBits value;
	while (iter.next(key, value))
	{
		if (!first)
		{
			if (pretty_)
			{
				writeConstString(",\n\t\t\t\t");
			}
			else
			{
				writeByte(',');
			}
		}
		else
		{
			first = false;
		}
		writeByte('\"');
		writeJsonEscapedString(key);
		if (pretty_)
		{
			writeConstString("\": ");
		}
		else
		{
			writeConstString("\":");
		}
		writeTagValue(iter.tags(), value, iter.strings());
	}
	if (pretty_)
	{
		writeConstString("\n\t\t\t}\n");
	}
	else
	{
		writeByte('}');
	}
}


void GeoJsonWriter::writeHeader()
{
	if (pretty_)
	{
		writeConstString(
			"{\n"
			"\t\"type\": \"FeatureCollection\",\n"
			"\t\"generator\": \"geodesk-py/"
			GEODESK_VERSION
			"\",\n"
			"\t\"features\": [\n");
	}
	else
	{
		writeConstString(
			"{\"type\":\"FeatureCollection\",\"generator\":\"geodesk-py/"
			GEODESK_VERSION
			"\",\"features\":[");
	}
}

void GeoJsonWriter::writeFooter()
{
	if (pretty_)
	{
		writeConstString("\n\t]\n}");
	}
	else
	{
		writeConstString("]}");
	}
}


void GeoJsonWriter::writeId(FeatureRef feature)
{
	if (pretty_)
	{
		writeConstString("\"id\": \"");
	}
	else
	{
		writeConstString("\"id\":\"");
	}
	writeByte("NWRX"[feature.typeCode()]);
	formatInt(feature.id());		// TODO: rename, is long in reality
	writeConstString("\",");
}

void GeoJsonWriter::writeNodeGeometry(NodeRef node)
{
	writeConstString("\"Point\",\"coordinates\":");
	writeCoordinate(node.xy());
}

void GeoJsonWriter::writeWayGeometry(WayRef way)
{
	if (way.isArea())
	{
		writeConstString("\"Polygon\",\"coordinates\":");
	}
	else
	{
		writeConstString("\"LineString\",\"coordinates\":");
	}
	writeWayCoordinates(way, way.isArea());
}

void GeoJsonWriter::writeAreaRelationGeometry(FeatureStore* store, RelationRef relation)
{
	Polygonizer polygonizer;
	polygonizer.createRings(store, relation);
	polygonizer.assignAndMergeHoles();
	const Polygonizer::Ring* ring = polygonizer.outerRings();
	int count = ring ? (ring->next() ? 2 : 1) : 0;
	if (count > 1)
	{
		writeConstString("\"MultiPolygon\",\"coordinates\":");
	}
	else
	{
		writeConstString("\"Polygon\",\"coordinates\":");
	}
	if (count == 0)
	{
		writeConstString("[]");
	}
	else
	{
		writePolygonizedCoordinates(polygonizer);
	}
}


void GeoJsonWriter::writeGeometry(FeatureStore* store, FeatureRef feature)
{
	writeConstString("{\"type\":");
	if (pretty_) writeByte(' ');
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
			writeConstString("\"GeometryCollection\",\"geometries\":[");
			writeMemberGeometries(store, relation, guard);
			writeByte(']');
		}
	}
	writeByte('}');
}


void GeoJsonWriter::writeMemberGeometries(FeatureStore* store, RelationRef relation, RecursionGuard& guard)
{
	// TODO
}


void GeoJsonWriter::writeFeature(FeatureStore* store, FeatureRef feature)
{
	TagIterator tagIter(feature.tags(), store->strings());
	if (pretty_)
	{
		if (!firstFeature_) writeConstString(",\n");
		writeConstString(
			"\t\t{\n"
			"\t\t\t\"type\": \"Feature\",\n\t\t\t");
		writeId(feature);
		writeByte('\n');
		// TODO: bbox
		writeConstString(
			"\t\t\t\"geometry\": ");
		writeGeometry(store, feature);
		writeConstString(
			",\n"
			"\t\t\t\"properties\": ");
		writeTags(tagIter);
		writeConstString(
			"\t\t}");
	}
	else
	{
		if (!firstFeature_) writeByte(',');
		writeConstString("{\"type\":\"Feature\",");
		writeId(feature);
		// TODO: bbox
		writeConstString("\"geometry\":");
		writeGeometry(store, feature);
		writeConstString(",\"properties\":");
		writeTags(tagIter);
		writeByte('}');
	}
	firstFeature_ = false;
}

