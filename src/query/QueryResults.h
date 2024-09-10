// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cstdint>
#include <common/util/DataPtr.h>

struct QueryResults;

struct QueryResultsHeader
{
    QueryResults* next;
    DataPtr pTile;
    uint32_t count;
};

struct QueryResults : public QueryResultsHeader
{
    static const uint32_t DEFAULT_BUCKET_SIZE = 256;
    static const uint32_t POTENTIAL_DUPLICATE = 0x8000'0000;

    static QueryResultsHeader EMPTY_HEADER;
    static QueryResults* const EMPTY;

    bool isFull() const
    {
        return count == DEFAULT_BUCKET_SIZE;
    }

    uint32_t items[DEFAULT_BUCKET_SIZE];
};


