// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "QueryResults.h"
#include "feature/types.h"
#include "filter/Filter.h"
#include "match/Matcher.h"
#include <common/util/pointer.h>

class Query;

class TileQueryTask
{
public:
    TileQueryTask(Query* query, uint32_t tipAndFlags) :
        query_(query),
        tipAndFlags_(tipAndFlags),
        // fastFilterHint_(0),     // TODO: initialize
        results_(QueryResults::EMPTY)
    {
    }

    TileQueryTask() {} // TODO: Never used, exists only to satisfy compiler


    void operator()();

private:
    void searchNodeIndexes();
    void searchNodeRoot(pointer ppRoot);
    void searchNodeBranch(pointer p);
    void searchNodeLeaf(pointer p);
    void searchIndexes(FeatureIndexType indexType);
    void searchRoot(pointer ppRoot);
    void searchBranch(pointer p);
    void searchLeaf(pointer p);
    void addResult(uint32_t item);

    Query* query_;
    uint32_t tipAndFlags_;
    FastFilterHint fastFilterHint_;     // TODO: initialize
    pointer pTile_;
    // MatcherMethod method_;      // TODO: not needed if we use a single method for all types
    QueryResults* results_;
};



