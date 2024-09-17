// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <geos_c.h>
#include "feature/RelationPtr.h"
#include "feature/WayPtr.h"
#include "geom/Box.h"

class GeometryBuilder
{
public:
	static GEOSGeometry* buildFeatureGeometry(FeatureStore* store, FeaturePtr feature, GEOSContextHandle_t geosContext);
	static GEOSGeometry* buildNodeGeometry(NodePtr node, GEOSContextHandle_t geosContext);
	static GEOSGeometry* buildWayGeometry(FeaturePtr way, GEOSContextHandle_t geosContext);
	static GEOSGeometry* buildAreaRelationGeometry(FeatureStore* store, RelationPtr relation, GEOSContextHandle_t geosContext);
	static GEOSGeometry* buildRelationGeometry(FeatureStore *store, RelationPtr relation, GEOSContextHandle_t geosContext);
	static GEOSGeometry* buildPointGeometry(int32_t x, int32_t y, GEOSContextHandle_t geosContext);
	static GEOSGeometry* buildBoxGeometry(const Box& box, GEOSContextHandle_t geosContext);
};


class RelationGeometryBuilder
{
public:
	RelationGeometryBuilder(FeatureStore* store, RelationPtr relation, GEOSContextHandle_t geosContext);
	GEOSGeometry* build();

private:
	void gatherMembers(RelationPtr relation);

	FeatureStore* store_;
	GEOSContextHandle_t context_;
	RecursionGuard guard_;
	std::vector< GEOSGeometry*> geoms_;
};
