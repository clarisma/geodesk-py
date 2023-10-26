// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <geos_c.h>
#include "feature/Relation.h"
#include "geom/mc/MCIndexBuilder.h"


class PreparedFilterFactory
{
public:
	const Filter* forFeature(FeatureStore* store, FeatureRef feature);
	const Filter* forGeometry(GEOSContextHandle_t geosContext, GEOSGeometry* geom);
	const Filter* forBox(const Box& box);
	virtual const Filter* forCoordinate(Coordinate point) { return nullptr; };

	const Box& bounds() const { return bounds_; }
	MCIndex buildIndex() { return indexBuilder_.build(bounds_); }

protected:
	virtual const Filter* forPolygonal() { return nullptr; };
	virtual const Filter* forLineal() { return nullptr; };
	virtual const Filter* forPuntal() { return nullptr; };
	virtual const Filter* forNonAreaRelation(FeatureStore* store, RelationRef feature)
	{
		return nullptr;
	};
	virtual const Filter* forGeometryCollection(GEOSContextHandle_t geosContext, GEOSGeometry* geom)
	{
		return nullptr;
	};

private:
	Box bounds_;
	MCIndexBuilder indexBuilder_;
};
