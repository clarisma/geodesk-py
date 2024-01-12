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
	void writeAnonymousNodeNode(Coordinate point) override;
	void writeHeader() override;
	void writeFooter() override;

	void writeTags(TagIterator& iter);

protected:
	void writeId(FeatureRef feature);
	void writeNodeGeometry(NodeRef node) override;
	void writeWayGeometry(WayRef way) override;
	void writeAreaRelationGeometry(FeatureStore* store, RelationRef relation) override;
	void writeCollectionRelationGeometry(FeatureStore* store, RelationRef relation) override;
};
