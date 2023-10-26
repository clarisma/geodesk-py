// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "Polygonizer.h"
#include "Ring.h"
#include "geom/LineSegment.h"
#include <unordered_map>

class Polygonizer::RingMerger
{
public:
    RingMerger(Arena& arena) :
        arena_(arena),
        firstValid_(nullptr) {}

    Ring* mergeRings(Ring* first);

private:
    class EdgeTracker
    {
    public:
        EdgeTracker(size_t capacity)
        {
            counts_.reserve(capacity);
        }

        void add(Coordinate start, Coordinate end)
        {
            LineSegment s(start, end);
            s.normalize();
            counts_[s]++;
        }

        bool isDuplicated(Coordinate start, Coordinate end)
        {
            LineSegment s(start, end);
            s.normalize();
            assert(counts_[s] > 0);
            return counts_[s] > 1;
        }

    private:
        std::unordered_map<LineSegment, int> counts_;
    };

    class SegmentCollector
    {
    public:
        SegmentCollector() : first_(nullptr), count_(0) {}

        void add(Segment* seg)
        {
            seg->status = Segment::SEGMENT_UNASSIGNED;
            seg->backward = false;
            seg->next = first_;
            first_ = seg;
            count_++;
        }

        /**
         * Adds all segments in a chain start with `start` up to
         * (but excluding) `end`.
         */
        void addChain(Segment* start, Segment* end)
        {
            Segment* p = start;
            while (p != end)
            {
                assert(p);
                Segment* next = p->next;    // add() changes next
                add(p);
                p = next;
            }
        }

        Segment* segments() const { return first_; }
        int count() const { return count_; }

    private:
        Segment* first_;
        int count_;
    };

    void addValid(Ring* ring)
    {
        ring->number_ = firstValid_ ? (firstValid_->number() + 1) : 1;
        ring->next_ = firstValid_;
        firstValid_ = ring;
    }

    void performMerge(Ring* first, int vertexCount);
    
    Arena& arena_;
    Ring* firstValid_;
};