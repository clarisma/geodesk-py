// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "FeatureWriter.h"

class WktWriter : public FeatureWriter
{
public:
	WktWriter(Buffer* buf); 

	void writeFeature(FeatureStore* store, FeatureRef feature) override;
	void writeAnonymousNodeNode(Coordinate point) override;
	void writeHeader() override;
	void writeFooter() override;

protected:
	void writeNodeGeometry(NodeRef node) override;
	void writeWayGeometry(WayRef way) override;
	void writeAreaRelationGeometry(FeatureStore* store, RelationRef relation) override;
	void writeCollectionRelationGeometry(FeatureStore* store, RelationRef relation) override;
};