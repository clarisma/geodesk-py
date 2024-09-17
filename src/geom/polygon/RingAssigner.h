// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "Polygonizer.h"
#include "Ring.h"

class Polygonizer::RingAssigner
{
public:
    /**
     * Matches an inner ring to an outer ring. The candidate outer rings should
     * ideally be arranged in the order of descending test cost, i.e. the largest
     * outer ring should come first. 
     * 
     * If only one outer ring is a possible candidate based on a cheap bounding-box
     * test, we assign the inner ring to it; if there is more than one, we have to
     * perform a proper point-in-polygon test.
     * 
     * This function will always assign the inner ring to an outer ring (by default,
     * the first ring in the list); it does not check the putative multipolygon
     * for validity.
     */
    static void assignRing(Ring** outerRings, int outerRingCount, Ring* inner)
    {
        Ring* tentativeOuter = nullptr;
        for (int i = outerRingCount - 1; i > 0; i--)
        {
            Ring* tryOuter = outerRings[i];

            // First, check if it is even possible for the outer to
            // contain the inner based on their bounding boxes
            if (tryOuter->containsBoundsOf(inner))
            {
                // Now check if the tentative outer-ring candidate
                // definitely contains the inner
                if (tentativeOuter && tentativeOuter->contains(inner))
                {
                    tentativeOuter->addInner(inner);
                    return;
                }
                tentativeOuter = tryOuter;
            }
        }
        // If the choice is down to one tentative outer and the first
        // outer in the list (presumably the largest and hence most 
        // expensive to check), perofmr a proper containment test
        // on the tentative candidate
        // (Remember, we skipped the bbox calculation for the largest
        // outer to save costs)
        if (tentativeOuter && tentativeOuter->contains(inner))
        {
            tentativeOuter->addInner(inner);
            return;
        }

        // If no other outer rings contain the inner, the largest ring 
        // is the outer by default
        outerRings[0]->addInner(inner);
    }

    static void assignRings(Ring* firstOuter, Ring* firstInner, Arena& arena)
    {
        // Build an array of rings, with the biggest one first
        // (We use vertex count as a proxy for "big")

        int outerCount = firstOuter->number();
        Ring** outerRings = arena.allocArray<Ring*>(outerCount);
        int maxVertexCount = 0;
        int biggestRing = 0;

        Ring* p = firstOuter;
        for (int i = 0; i < outerCount; i++)
        {
            int vertexCount = p->vertexCount();
            if (vertexCount > maxVertexCount)
            {
                maxVertexCount = vertexCount;
                biggestRing = i;
            }
            outerRings[i] = p;
            p = p->next();
        }
        std::swap(outerRings[0], outerRings[biggestRing]);

        // Calculate bboxes of all rings, except for the largest

        for (int i = 1; i < outerCount; i++)
        {
            outerRings[i]->calculateBounds();
        }
        
        Ring* inner = firstInner;
        do
        {
            inner->calculateBounds();
            Ring* next = inner->next();
            // assignRing may change next because it re-chains the rings,
            // so we retrieve the pointer beforehand
            assignRing(outerRings, outerCount, inner);
            inner = next;
        }
        while (inner);
    }

};

