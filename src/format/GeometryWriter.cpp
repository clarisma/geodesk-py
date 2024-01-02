// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "GeometryWriter.h"
#include "feature/polygon/Polygonizer.h"
#include "feature/polygon/Ring.h"
#include "feature/polygon/RingCoordinateIterator.h"
#include "geom/Mercator.h"
#include "geom/geos/GeosCoordinateIterator.h"

void GeometryWriter::writeCoordinate(Coordinate c)
{
	if (coordStartChar_) writeByte(coordStartChar_);
	double lon = Mercator::lonFromX(c.x);
	double lat = Mercator::latFromY(c.y);
	formatDouble(latitudeFirst_ ? lat : lon, precision_);
	writeByte(coordValueSeparatorChar_);
	formatDouble(latitudeFirst_ ? lon : lat, precision_);
	if (coordEndChar_) writeByte(coordEndChar_);
}


void GeometryWriter::writeCoordinateSegment(bool isFirst, const Coordinate* p, size_t count)
{
	const Coordinate* end;
	end = p + count;
	while (p < end)
	{
		if (!isFirst) writeByte(',');  // TODO: always comma for all formats?
		isFirst = false;
		writeCoordinate(*p++);
	}
}

void GeometryWriter::writeCoordSequence(GEOSContextHandle_t context, const GEOSCoordSequence* coords)
{
	GeosCoordinateIterator iter(context, coords);
    writeCoordinates(iter);
}


void GeometryWriter::writePointCoordinates(GEOSContextHandle_t context, const GEOSGeometry* point)
{
    const GEOSCoordSequence* coords = GEOSGeom_getCoordSeq_r(context, point);
    double x = 0;
    double y = 0;
    GEOSCoordSeq_getXY_r(context, coords, 0, &x, &y);
    writeCoordinate(Coordinate((int32_t)std::round(x), (int32_t)std::round(y)));
}


void GeometryWriter::writeLineStringCoordinates(GEOSContextHandle_t context, const GEOSGeometry* line)
{
    writeCoordSequence(context, GEOSGeom_getCoordSeq_r(context, line));
}


void GeometryWriter::writePolygonCoordinates(GEOSContextHandle_t context, const GEOSGeometry* polygon)
{
    writeByte(coordGroupStartChar_);
    const GEOSGeometry* exteriorRing = GEOSGetExteriorRing_r(context, polygon);
    writeCoordSequence(context, GEOSGeom_getCoordSeq_r(context, exteriorRing));

    int numInteriorRings = GEOSGetNumInteriorRings_r(context, polygon);
    for (int i = 0; i < numInteriorRings; i++)
    {
        const GEOSGeometry* interiorRing = GEOSGetInteriorRingN_r(context, polygon, i);
        writeByte(',');  // TODO: always comma for all formats?
        writeCoordSequence(context, GEOSGeom_getCoordSeq_r(context, interiorRing));
    }
    writeByte(coordGroupEndChar_);
}


void GeometryWriter::writeMultiPolygonCoordinates(GEOSContextHandle_t context, const GEOSGeometry* multiPolygon)
{
    writeMultiGeometryCoordinates(context, multiPolygon,
        [this](GEOSContextHandle_t ctx, const GEOSGeometry* g)
        {
            writePolygonCoordinates(ctx, g);
        });
}

void GeometryWriter::writeGeometryCoordinates(
	GEOSContextHandle_t context, int type, const GEOSGeometry* geom)
{
    switch (type) 
    {
    case GEOS_POINT:
        writePointCoordinates(context, geom);
        break;
    case GEOS_LINESTRING:
    case GEOS_LINEARRING:
        writeLineStringCoordinates(context, geom);
        break;
    case GEOS_POLYGON:
        writePolygonCoordinates(context, geom);
        break;
    case GEOS_MULTIPOINT:
        writeMultiGeometryCoordinates(context, geom,
            [this](GEOSContextHandle_t ctx, const GEOSGeometry* g)
            {
                writePointCoordinates(ctx, g);
            });
        break;
    case GEOS_MULTILINESTRING:
        writeMultiGeometryCoordinates(context, geom, 
            [this](GEOSContextHandle_t ctx, const GEOSGeometry* g)
            {
                writeLineStringCoordinates(ctx, g);
            });
        break;
    case GEOS_MULTIPOLYGON:
        writeMultiPolygonCoordinates(context, geom);
        break;
    }
    // (heterogeneous collections need special handling by caller)
}



void GeometryWriter::writeWayCoordinates(WayRef way)
{
    WayCoordinateIterator iter(way);
    // TODO: Leaflet doesn't need duplicate end coordinate for polygons
    bool isFirst = true;
    writeByte(coordGroupStartChar_);
    for (;;)
    {
        Coordinate c = iter.next();
        if (c.isNull()) break;
        if (!isFirst) writeByte(',');  // TODO: always comma for all formats?
        isFirst = false;
        writeCoordinate(c);
    }
    writeByte(coordGroupEndChar_);
}



void GeometryWriter::writePolygonizedCoordinates(const Polygonizer& polygonizer)
{
    const Polygonizer::Ring* first = polygonizer.outerRings();
    assert (first);
    if (first->next()) writeByte(coordGroupStartChar_);
    const Polygonizer::Ring* ring = first;
    bool isFirst = true;
    do
    {
        if (!isFirst) writeByte(',');  // TODO: always comma for all formats?
        isFirst = false;
        writeByte(coordGroupStartChar_);
        RingCoordinateIterator iter(ring);
        writeCoordinates(iter);
        const Polygonizer::Ring* inner = ring->firstInner();
        while (inner)
        {
            writeByte(',');  // TODO: always comma for all formats?
            RingCoordinateIterator iterInner(inner);
            writeCoordinates(iterInner);
            inner = inner->next();
        }
        writeByte(coordGroupEndChar_);
        ring = ring->next();
    }
    while (ring);
    if (first->next()) writeByte(coordGroupEndChar_);
}
