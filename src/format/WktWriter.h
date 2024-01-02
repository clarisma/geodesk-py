// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "FeatureWriter.h"

class WktWriter : public FeatureWriter
{
public:
	WktWriter(Buffer* buf); 

	void writeFeature(FeatureStore* store, FeatureRef feature) override;
	void writeHeader() override;
	void writeFooter() override;

protected:
	void writeGeometry(FeatureStore* store, FeatureRef feature);
	void writeNodeGeometry(NodeRef node);
	void writeWayGeometry(WayRef way);
	void writeAreaRelationGeometry(FeatureStore* store, RelationRef relation);
	void writeMemberGeometries(FeatureStore* store, RelationRef relation, RecursionGuard& guard);
};