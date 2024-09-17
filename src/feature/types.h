// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <Python.h>
#include <cstdint>
#include "feature/Tip.h"

// TODO: make enum class
// TODO: move to public API
enum FeatureType
{
    NODE = 0,
    WAY = 1,
    RELATION = 2
};

enum FeatureIndexType
{
    NODES = 0,
    WAYS = 1,
    AREAS = 2,
    RELATIONS = 3
};

class IndexBits
{
public:
    static uint32_t fromCategory(int category)
    {
        return category == 0 ? 0 : (1 << (category - 1));
    }
};

enum FeatureFlags
{
    LAST_SPATIAL_ITEM = 1,
    AREA = 1 << 1,
    RELATION_MEMBER = 1 << 2,
    WAYNODE = 1 << 5,
    MULTITILE_WEST = 1 << 6,
    MULTITILE_NORTH = 1 << 7
};


enum MemberFlags
{
    LAST = 1,
    FOREIGN = 2,
    DIFFERENT_ROLE = 4,
    DIFFERENT_TILE = 8,     
        // TODO: This will change in 2.0 for relation tables and feature-node tables
        // (moves to Bit 2 == value 4, to accommodate more TEX bits)
    WIDE_NODE_TEX = 8,
    WIDE_RELATION_TEX = 8,
    WIDE_MEMBER_TEX = 16,
};

namespace FeatureConstants
{
    static const Tip START_TIP(0x4000);     
        // TODO: move to Tip? No, not really a characteristic of TIP,
        //  it is driven by the encoding used by member/node/relation tables
    static const int MAX_COMMON_KEY = (1 << 13) - 2;
    static const int MAX_COMMON_ROLE = (1 << 15) - 1;
};

