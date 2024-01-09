// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <algorithm>
#include <cstdint>
#include <string>

struct OsmStatistics
{
    OsmStatistics()
    {
        memset(this, 0, sizeof(OsmStatistics));
    }

    OsmStatistics& operator+=(const OsmStatistics& other)
    {
        nodeCount     += other.nodeCount;
        wayCount      += other.wayCount;
        relationCount += other.relationCount;
        memberCount   += other.memberCount;
        tagCount      += other.tagCount;
        maxNodeId      = std::max(maxNodeId, other.maxNodeId);
        maxWayId       = std::max(maxWayId, other.maxWayId);
        maxRelationId  = std::max(maxRelationId, other.maxRelationId);
        return *this;
    }

    uint64_t nodeCount;
    uint64_t wayCount;
    uint64_t relationCount;
    uint64_t memberCount;
    uint64_t tagCount;
    uint64_t maxNodeId;
    uint64_t maxWayId;
    uint64_t maxRelationId;
};
