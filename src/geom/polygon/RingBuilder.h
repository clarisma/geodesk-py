// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "Polygonizer.h"

class Polygonizer::RingBuilder
{
public:
    RingBuilder(int segmentCount, Segment* firstSegment, Arena& arena);
    Ring* build();

private:
    struct Candidate
    {
        int segmentNumber;
        int nextCandidate;
    };

    inline int slotOf(Coordinate c) const
    {
        return (c.x ^ c.y) & (tableSize_ - 1);
    }

    void addToTable(Coordinate c, int segmentNumber)
    {
        int slot = slotOf(c);
        candidates_[candidateCount_].segmentNumber = segmentNumber;
        candidates_[candidateCount_].nextCandidate = lookupTable_[slot];
        lookupTable_[slot] = candidateCount_++;
    }

    Segment* findNeighbor(Segment* current);
    static int markAndCount(Segment* segment);

    Arena& arena_;
    uint32_t segmentCount_;
    Segment** segments_;
    Candidate* candidates_;
    int* lookupTable_;
    uint32_t tableSize_;
    uint32_t candidateCount_;
};

