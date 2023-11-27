// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "RingBuilder.h"
#include "Segment.h"
#include "geom/Coordinate.h"
#include <common/util/Bits.h>


/**
 * In the constructor, we create a custom hashtable that allows the lookup of
 * segments by their start and end coordinates. For this, we use three arrays:
 *
 * 1. segments_:     an array of all the Segments
 * 2. candidates_:   an intermediate structure that simulates a linked list of
 *                   entries at a hash position
 * 3. lookupTable_:  The hashtable itself, which translates the hash of a
 *                   coordinate (modulo tableSize_) into the number of the
 *                   first entry in candidates_, or -1 if there is no segment
 *                   that has the coordinate as start or end point (which means
 *                   at least part of the geometry is broken)
 *
 * Remember to check the actual coordinates of the indexes segments, as the
 * multiple coordinates may lead to the same candidate chain due to hash
 * collisions.
 */
Polygonizer::RingBuilder::RingBuilder(int segmentCount, Segment* firstSegment, Arena& arena) :
    arena_(arena),
    segmentCount_(segmentCount),
    candidateCount_(0)
{
    segments_ = arena.allocArray<Segment*>(segmentCount);
    // tableSize_ = (((uint32_t)0xffff'ffff) >> Bits::numberOfLeadingZeros(segmentCount - 1)) + 1;
    tableSize_ = (((uint32_t)0xffff'ffff) >> Bits::countLeadingZeros32(
        (segmentCount - 1) | 1)) + 1;
    assert(tableSize_ > 0);

    lookupTable_ = arena.allocArray<int>(tableSize_);
    std::fill(lookupTable_, lookupTable_ + tableSize_, -1);
    // Set each slot in the hashtable to -1 ("no entry")
    candidates_ = arena.allocArray<Candidate>(segmentCount * 2);
    // each segment is entered twice into the candidate list (for first and last vertex)
    int i = 0;
    Segment* seg = firstSegment;
    while (seg)
    {
        segments_[i] = seg;
        addToTable(seg->coords[0], i);
        addToTable(seg->coords[seg->vertexCount - 1], i);
        seg = seg->next;
        i++;
    }
    assert(i == segmentCount);
    assert(candidateCount_ == segmentCount * 2);
}


/**
 * Finds the first segment which:
 *  - has not been assigned;
 *  - is not the current segment; and
 *  - whose start or end point matches the given coordinates, which
 *    are the start point of the current segment
 *
 * @param table
 * @param current
 * @return
 */
Polygonizer::Segment* Polygonizer::RingBuilder::findNeighbor(Segment* current)
{
    Coordinate c = current->coords[current->backward ? (current->vertexCount - 1) : 0];
    int candidateNumber = lookupTable_[slotOf(c)];
    while (candidateNumber >= 0)
    {
        Candidate& candidate = candidates_[candidateNumber];
        Segment* seg = segments_[candidate.segmentNumber];
        if (seg->status < Segment::SEGMENT_ASSIGNED && seg != current)
        {
            if (seg->coords[0] == c)
            {
                seg->backward = true;
                return seg;
            }
            if (seg->coords[seg->vertexCount - 1] == c)
            {
                seg->backward = false;
                return seg;
            }
        }
        candidateNumber = candidate.nextCandidate;
    }
    return nullptr;
}

/**
 * Tallies the total number of vertexes in a segment chain and marks
 * aech segment as assigned.
 *
 * @param segment the first segment of a completed ring
 */
int Polygonizer::RingBuilder::markAndCount(Segment* segment)
{
    int vertexCount = segment->vertexCount;
    for (;;)
    {
        segment->status = Segment::SEGMENT_ASSIGNED;
        segment = segment->next;
        if (!segment) return vertexCount;
        vertexCount += segment->vertexCount - 1;
        // -1 because the first vertex of the segment is already 
        // part of the preceding segment
    }
}


Polygonizer::Ring* Polygonizer::RingBuilder::build()
{
    int ringCount = 0;
    Ring* rings = nullptr;

    for (unsigned int i = 0; i < segmentCount_; i++)
    {
        Segment* seg = segments_[i];
        if (seg->status != Segment::SEGMENT_UNASSIGNED) continue;
        seg->backward = false;
        seg->next = nullptr;
        if (seg->isClosed())
        {
            seg->status = Segment::SEGMENT_ASSIGNED;
            rings = createRing(seg->vertexCount, seg, rings, arena_);
            continue;
        }
        seg->status = Segment::SEGMENT_TENTATIVE;
        for (; ; )
        {
            Segment* candidate = findNeighbor(seg);
            if (!candidate)
            {
                seg->status = Segment::SEGMENT_DANGLING;
                seg = seg->next;
            }
            else if (candidate->status == Segment::SEGMENT_TENTATIVE)
            {
                Segment* nextSegment = candidate->next;
                candidate->next = nullptr;
                rings = createRing(markAndCount(seg), seg, rings, arena_);
                seg = nextSegment;
            }
            else if (candidate->isClosed())
            {
                // TODO: is this really needed?
                // Wouldn't the check above pick up this single-segment ring?
                candidate->status = Segment::SEGMENT_ASSIGNED;
                candidate->next = nullptr;
                rings = createRing(candidate->vertexCount, candidate, rings, arena_);
                continue;
            }
            else
            {
                candidate->status = Segment::SEGMENT_TENTATIVE;
                candidate->next = seg;
                seg = candidate;
                continue;
            }
            if (!seg) break;
        }
    }
    return rings;
}

