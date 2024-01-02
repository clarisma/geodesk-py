// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <utility>

namespace sorting
{
    template <typename ItemType, typename KeyType, typename IndexType, IndexType Sentinel>
    void rearrage(ItemType* items, std::pair< KeyType,IndexType>* indices, size_t count)
    {
        for (size_t i = 0; i < count; ++i)
        {
            if (indices[i].second == -1) continue;  // Skip if this index is part of a previously processed cycle

            size_t j = i;
            ItemType tempItem = items[i];

            while (indices[j].second != Sentinel)
            {
                size_t next_j = indices[j].second;
                if (next_j == i)
                {
                    items[j] = tempItem;
                    indices[j].second = Sentinel;  // Mark as visited
                    break;
                }
                else
                {
                    items[j] = items[next_j];
                    indices[j].second = Sentinel;  // Mark as visited
                }
                j = next_j;
            }
        }
    }
}

