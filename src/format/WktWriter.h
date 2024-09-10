// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "FeatureWriter.h"

class WktWriter : public FeatureWriter
{
public:
	WktWriter(Buffer* buf); 

	void writeFeature(FeatureStore* store, FeaturePtr feature) override;
	void writeAnonymousNodeNode(Coordinate point) override;
	void writeHeader() override;
	void writeFooter() override;

protected:
	void writeNodeGeometry(NodePtr node) override;
	void writeWayGeometry(WayPtr way) override;
	void writeAreaRelationGeometry(FeatureStore* store, RelationPtr relation) override;
	void writeCollectionRelationGeometry(FeatureStore* store, RelationPtr relation) override;
};