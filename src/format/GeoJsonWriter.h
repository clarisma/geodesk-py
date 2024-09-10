// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "FeatureWriter.h"
#include "feature/TagIterator.h"

class GeoJsonWriter : public FeatureWriter
{
public:
	GeoJsonWriter(Buffer* buf) : FeatureWriter(buf) {}

	void linewise(bool b) { linewise_ = b; }

	void writeFeature(FeatureStore* store, FeaturePtr feature) override;
	void writeAnonymousNodeNode(Coordinate point) override;
	void writeHeader() override;
	void writeFooter() override;

	void writeTags(TagIterator& iter);

protected:
	// void writeId(FeatureRef feature);
	void writeNodeGeometry(NodePtr node) override;
	void writeWayGeometry(WayPtr way) override;
	void writeAreaRelationGeometry(FeatureStore* store, RelationPtr relation) override;
	void writeCollectionRelationGeometry(FeatureStore* store, RelationPtr relation) override;

	bool linewise_;
};
