// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "RingMerger.h"
#include "RingBuilder.h"
#include "Segment.h"
#include "geom/LineSegment.h"
#include <unordered_map>
#include <common/util/log.h>

// See also alternative approach that counts coords (but resolves fewer issues,
// and possibly crreates new ones)

Polygonizer::Ring* Polygonizer::RingMerger::mergeRings(Ring* first)
{
    // First, we create an array of all rings, sorted by minX of their bbox
    // If the bbox hasn't been calculated (e.g. because we didn't need to 
    // assign rings as there is only one outer), we'll calculate it during
    // this step

    int ringCount = first->number();
    assert(ringCount > 1);      
        // don't call this method unless there are 2 or more inner rings

    Ring** rings = arena_.allocArray<Ring*>(ringCount);
    Ring* ring = first;
    for (int i = 0; i < ringCount; i++)
    {
        assert(ring->number() != 0);
        if (ring->bounds_.isEmpty()) ring->calculateBounds();
        rings[i] = ring;
        // LOG("#%d: Ring of way/%ld at %d", i, ring->firstSegment_->way.id(), ring->bounds_.minX());
        ring = ring->next();
    }
    std::sort(rings, rings + ringCount, Ring::compareMinX);

    // Now, we check which rings potentially touch other rings based
    // on their bounding boxes. Since we scan the minX-ordered array
    // from left to right, we can stop once we see a candidate whose 
    // minX > maxX of the current ring. 
    // We set the number of each potential merge candidate to 0 and
    // add it to a list ("live" rings always have a non-zero number)

    Ring* firstMergeCandidate = nullptr;
    int vertexCount = 0;

    for (int i = 0; i < ringCount; i++)
    {
        Ring* ring = rings[i];
        for (int i2 = i + 1; i2 < ringCount; i2++)
        {
            Ring* otherRing = rings[i2];
            if (otherRing->bounds_.minX() > ring->bounds_.maxX()) break;
            // All further rings have higher minX as well, so they
            // can't possibly overlap with this ring

            if (otherRing->bounds_.minY() <= ring->bounds_.maxY() &&
                otherRing->bounds_.maxY() >= ring->bounds_.minY())
            {
                // The rings' bboxes overlap
                ring->number_ = 0;
                otherRing->number_ = 0;
            }
        }
        if (ring->number() == 0)
        {
            // LOG("Ring of way/%ld must be checked...", ring->firstSegment_->way.id());
            ring->next_ = firstMergeCandidate;
            firstMergeCandidate = ring;
            vertexCount += ring->vertexCount();
        }
        else
        {
            // LOG("Ring of way/%ld is valid.", ring->firstSegment_->way.id());
            addValid(ring);
        }
    }

    if (firstMergeCandidate)
    {
        performMerge(firstMergeCandidate, vertexCount);
    }
    return firstValid_; 
}



void Polygonizer::RingMerger::performMerge(Ring* first, int vertexCount)
{
    EdgeTracker edges(vertexCount);
        // we use double the vertexCount to get a decent 
        // pre-allocated hashmap size

    // (Remember that ring numbers are all zero at this point, 
    // so we can't use them to establish a ring count)

    Ring* ring = first;
    do
    {
        assert(ring->number() == 0);
        Segment* seg = ring->firstSegment_;
        do
        {
            for (int i = 0; i < seg->vertexCount - 1; i++)
            {
                edges.add(seg->coords[i], seg->coords[i+1]);
            }
            seg = seg->next;
        }
        while (seg);
        ring = ring->next();
    }
    while (ring);

    // Now we check if edges are shared by any rings
    // Rings without touching edges are added to the list of valid rings
    // Invalid rings are broken into their constituent segments
    
    SegmentCollector segments;
    ring = first;
    do
    {
        bool isValidRing = true;
        Segment* seg = ring->firstSegment_;
        do
        {
            Segment* next = seg->next;
            int firstValidVertex = 0;
            for (int i = 0; i < seg->vertexCount - 1; i++)
            {
                if (edges.isDuplicated(seg->coords[i], seg->coords[i + 1]))
                {
                    if (isValidRing)
                    {
                        LOG("Segment of way/%llu overlaps.", seg->way.id());
                        // If this is the first overlapping edge in this ring,
                        // add any segments preceding the current to the collection
                        segments.addChain(ring->firstSegment_, seg);
                    }
                    if (firstValidVertex < i)
                    {
                        // Create a fragment from the preceding part of the segment
                        // where no edges overlap
                        segments.add(seg->createFragment(firstValidVertex, i+1, arena_));
                    }
                    firstValidVertex = i + 1;
                    isValidRing = false;
                }
            }
            if (!isValidRing && firstValidVertex < seg->vertexCount-1)
            {
                if (firstValidVertex == 0)
                {
                    // There were no overlapping edges in the segment, so we 
                    // can reuse the segment as-is
                    segments.add(seg);
                }
                else
                {
                    // otherwise, create a fragment based on the remaining
                    // non-overlapping parts of the segment
                    segments.add(seg->createFragment(firstValidVertex,
                        seg->vertexCount, arena_));
                }
            }
            seg = next;     
                // can't use seg->next because it may have changed because of add
        }
        while (seg);
        Ring* next = ring->next();  // addValid changes next, so get it now
        if (isValidRing) addValid(ring);
        ring = next;
    }
    while (ring);

    // Re-polygonize the segments of invalid rings

    int segmentCount = segments.count();
    if (segmentCount > 0)
    {
        RingBuilder builder(segmentCount, segments.segments(), arena_);
        Ring* ring = builder.build();
        while (ring)
        {
            Ring* next = ring->next();
            addValid(ring);
            ring = next;
        }
    }
}

