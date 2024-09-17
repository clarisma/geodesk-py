// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "feature/WayPtr.h"
#include "geom/Coordinate.h"
#include "GeometryWriter.h"

class FeatureWriter : public GeometryWriter
{
public:
	using WriteIdFunction = void(*)(FeatureWriter* writer, 
		FeatureStore* store, FeaturePtr feature, void* closure);

	FeatureWriter(Buffer* buf) : 
		GeometryWriter(buf),
		writeIdFunction_(writeDefaultId),
		writeIdClosure_(nullptr)
	{
	}
	virtual ~FeatureWriter() {};
	// Attention: The base classes don't have destructors

	void writeIdFunction(WriteIdFunction func, void* closure)
	{
		writeIdFunction_ = func;
		writeIdClosure_ = closure;
	}
	char quoteChar() const { return quoteChar_; }
	void pretty(bool b) { pretty_ = b; }
	void flush() { GeometryWriter::flush(); }	// TODO: needed?

	virtual void writeFeature(FeatureStore* store, FeaturePtr feature) = 0;
	virtual void writeAnonymousNodeNode(Coordinate point) = 0;
	virtual void writeHeader() {}
	virtual void writeFooter() {}

protected:
	// void writeWayCoordinates(WayRef way);
	void writeFeatureGeometry(FeatureStore* store, FeaturePtr feature);
	virtual void writeNodeGeometry(NodePtr way) = 0;
	virtual void writeWayGeometry(WayPtr way) = 0;
	virtual void writeAreaRelationGeometry(FeatureStore* store, RelationPtr feature) = 0;
	virtual void writeCollectionRelationGeometry(FeatureStore* store, RelationPtr feature) = 0;
	uint64_t writeMemberGeometries(FeatureStore* store, RelationPtr relation, RecursionGuard& guard);
	uint64_t writeMemberGeometries(FeatureStore* store, RelationPtr relation)
	{
		RecursionGuard guard(relation);
		return writeMemberGeometries(store, relation, guard);
	}

	void writeId(FeatureStore* store, FeaturePtr feature);
	static void writeDefaultId(FeatureWriter* writer,
		FeatureStore* store, FeaturePtr feature, void* closure);
	void writeTagValue(TagTablePtr tags, TagBits value, StringTable& strings);

	WriteIdFunction writeIdFunction_;
	void* writeIdClosure_;
	bool pretty_ = true;
	bool firstFeature_ = true;
	char quoteChar_ = '\"';
	// char featureGroupStartChar_ = ' ';
	// char featureGroupEndChar_ = ' ';
};
