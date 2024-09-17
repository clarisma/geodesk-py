// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "Polygonizer.h"
#include "PointInPolygon.h"
#include "Segment.h"

class Polygonizer::Ring
{
public:
    Ring(int vertexCount, Segment* firstSegment, Ring* next) :
        firstSegment_(firstSegment),
        firstInner_(nullptr),
        next_(next),
        number_(next ? (next->number_ + 1) : 1),
        vertexCount_(vertexCount) {}

    int number() const { return number_; };
    int vertexCount() const { return vertexCount_; };
    Ring* next() const { return next_; }
    Ring* firstInner() const { return firstInner_; }
    void calculateBounds();
    GEOSCoordSequence* createCoordSequence(GEOSContextHandle_t context);
    GEOSGeometry* createLinearRing(GEOSContextHandle_t context);
    GEOSGeometry* createPolygon(GEOSContextHandle_t context, Arena& arena);
    
    // Sort order (used by RingMerger)
    static bool compareMinX(const Ring* a, const Ring* b) 
    {
        return a->bounds_.minX() < b->bounds_.minX();
    }

private:
    bool containsBoundsOf(Ring* potentialInner)
    {
        return bounds_.containsSimple(potentialInner->bounds_);
    }

    /**
     * Tests if potentialInner lies definitely within this Ring
     * (assumes containsBoundsOf() has already been checked)
     */
    bool contains(Ring* potentialInner);
    PointInPolygon::Location locateCoordinate(Coordinate c);

    void addInner(Ring* inner)
    {
        inner->number_ = firstInner_ ? (firstInner_->number_ + 1) : 1;
        inner->next_ = firstInner_;
        firstInner_ = inner;
    }

    Segment* firstSegment_;
    Ring* firstInner_;
    Ring* next_;
    int number_;
    int vertexCount_;
    Box bounds_;

    friend class Polygonizer;
    friend class RingAssigner;
    friend class RingMerger;
    friend class RingCoordinateIterator;
};