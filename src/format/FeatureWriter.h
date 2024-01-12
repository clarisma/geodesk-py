// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "feature/Way.h"
#include "geom/Coordinate.h"
#include "GeometryWriter.h"

class FeatureWriter : public GeometryWriter
{
public:
	FeatureWriter(Buffer* buf) : GeometryWriter(buf) {}
	virtual ~FeatureWriter() {};
	// Attention: The base classes don't have destructors

	void pretty(bool b) { pretty_ = b; }
	void flush() { GeometryWriter::flush(); }

	virtual void writeFeature(FeatureStore* store, FeatureRef feature) = 0;
	virtual void writeAnonymousNodeNode(Coordinate point) = 0;
	virtual void writeHeader() {}
	virtual void writeFooter() {}

protected:
	// void writeWayCoordinates(WayRef way);
	void writeFeatureGeometry(FeatureStore* store, FeatureRef feature);
	virtual void writeNodeGeometry(NodeRef way) = 0;
	virtual void writeWayGeometry(WayRef way) = 0;
	virtual void writeAreaRelationGeometry(FeatureStore* store, RelationRef feature) = 0;
	virtual void writeCollectionRelationGeometry(FeatureStore* store, RelationRef feature) = 0;
	uint64_t writeMemberGeometries(FeatureStore* store, RelationRef relation, RecursionGuard& guard);
	uint64_t writeMemberGeometries(FeatureStore* store, RelationRef relation)
	{
		RecursionGuard guard(relation);
		return writeMemberGeometries(store, relation, guard);
	}

	void writeTagValue(TagsRef tags, TagBits value, StringTable& strings);

	bool pretty_ = true;
	bool firstFeature_ = true;
	// char featureGroupStartChar_ = ' ';
	// char featureGroupEndChar_ = ' ';
};
