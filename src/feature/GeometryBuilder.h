// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cmath>
#include <geos_c.h>
#include "Feature.h"
#include "Relation.h"
#include "Way.h"
#include "geom/Box.h"

class GeometryBuilder
{
public:
	static GEOSGeometry* buildFeatureGeometry(FeatureStore* store, const FeatureRef feature, GEOSContextHandle_t geosContext);
	static GEOSGeometry* buildNodeGeometry(const NodeRef node, GEOSContextHandle_t geosContext);
	static GEOSGeometry* buildWayGeometry(const FeatureRef way, GEOSContextHandle_t geosContext);
	static GEOSGeometry* buildAreaRelationGeometry(FeatureStore* store, const RelationRef relation, GEOSContextHandle_t geosContext);
	static GEOSGeometry* buildRelationGeometry(FeatureStore *store, const RelationRef relation, GEOSContextHandle_t geosContext);
	static GEOSGeometry* buildPointGeometry(int32_t x, int32_t y, GEOSContextHandle_t geosContext);
	static GEOSGeometry* buildBoxGeometry(const Box& box, GEOSContextHandle_t geosContext);
};


class RelationGeometryBuilder
{
public:
	RelationGeometryBuilder(FeatureStore* store, const RelationRef relation, GEOSContextHandle_t geosContext);
	GEOSGeometry* build();

private:
	void gatherMembers(const RelationRef relation);

	FeatureStore* store_;
	GEOSContextHandle_t context_;
	RecursionGuard guard_;
	std::vector< GEOSGeometry*> geoms_;
};
