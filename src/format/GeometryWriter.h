// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <common/util/BufferWriter.h>
#include <geos_c.h>
#include "feature/Way.h"
#include "feature/Relation.h"
#include "geom/Coordinate.h"
#include <functional>

class Polygonizer;

class GeometryWriter : public BufferWriter
{
public:
	GeometryWriter(Buffer* buf) : BufferWriter(buf) {}
	
protected:
	void writeCoordinate(Coordinate c);
	
	template<typename Iter>
	void writeCoordinates(Iter& iter)
	{
		bool isFirst = true;
		writeByte(coordGroupStartChar_);
		for (int count = iter.coordinatesRemaining(); count > 0; count--)
		{
			Coordinate c = iter.next();
			if (!isFirst) writeByte(',');  // TODO: always comma for all formats?
			isFirst = false;
			writeCoordinate(c);
		}
		writeByte(coordGroupEndChar_);
	}

	// ==== GEOS Geometries ====

	void writeCoordinateSegment(bool isFirst, const Coordinate* coords, size_t count);
	void writeCoordSequence(GEOSContextHandle_t context, const GEOSCoordSequence* coords);
	void writePointCoordinates(GEOSContextHandle_t context, const GEOSGeometry* point);
	void writeLineStringCoordinates(GEOSContextHandle_t context, const GEOSGeometry* line);
	void writePolygonCoordinates(GEOSContextHandle_t context, const GEOSGeometry* polygon);
	void writeMultiPolygonCoordinates(GEOSContextHandle_t context, const GEOSGeometry* multiPolygon);
	void writeGeometryCoordinates(GEOSContextHandle_t context, int type, const GEOSGeometry* geom);

	void writeMultiGeometryCoordinates(
		GEOSContextHandle_t context, const GEOSGeometry* multi, 
		std::function<void(GEOSContextHandle_t, const GEOSGeometry*)> writeFunc)
	{
		writeByte(coordGroupStartChar_);
		int count = GEOSGetNumGeometries_r(context, multi);
		for (int i = 0; i < count; i++) 
		{
			if(i > 0) writeByte(',');
			const GEOSGeometry* geom = GEOSGetGeometryN_r(context, multi, i);
			writeFunc(context, geom);
		}
		writeByte(coordGroupEndChar_);
	}

	// ==== Feature Geometries ====

	void writeWayCoordinates(WayRef way, bool group);
	void writePolygonizedCoordinates(const Polygonizer& polygonizer);

	int precision_ = 7;
	bool latitudeFirst_ = false;
	char coordValueSeparatorChar_ = ',';
	char coordStartChar_ = '[';
	char coordEndChar_ = ']';
	char coordGroupStartChar_ = '[';
	char coordGroupEndChar_ = ']';
};
