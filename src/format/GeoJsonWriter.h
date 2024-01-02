// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "FeatureWriter.h"
#include "feature/TagIterator.h"

class GeoJsonWriter : public FeatureWriter
{
public:
	GeoJsonWriter(Buffer* buf) : FeatureWriter(buf) {}

	void writeFeature(FeatureStore* store, FeatureRef feature) override;
	void writeHeader() override;
	void writeFooter() override;

	void writeTags(TagIterator& iter);

protected:
	void writeId(FeatureRef feature);
	void writeGeometry(FeatureStore* store, FeatureRef feature);
	void writeNodeGeometry(NodeRef node);
	void writeWayGeometry(WayRef way);
	void writeAreaRelationGeometry(FeatureStore* store, RelationRef relation);
	void writeMemberGeometries(FeatureStore* store, RelationRef relation, RecursionGuard& guard);
};
