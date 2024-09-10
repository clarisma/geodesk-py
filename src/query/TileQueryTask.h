// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "QueryResults.h"
#include "feature/types.h"
#include "filter/Filter.h"
#include "match/Matcher.h"
#include <common/util/DataPtr.h>

class Query;

class TileQueryTask
{
public:
    TileQueryTask(Query* query, uint32_t tipAndFlags, FastFilterHint fastFilterHint) :
        query_(query),
        tipAndFlags_(tipAndFlags),
        fastFilterHint_(fastFilterHint),     
        results_(QueryResults::EMPTY)
    {
    }

    TileQueryTask() {} // TODO: Never used, exists only to satisfy compiler


    void operator()();

private:
    void searchNodeIndexes();
    void searchNodeRoot(DataPtr ppRoot);
    void searchNodeBranch(DataPtr p);
    void searchNodeLeaf(DataPtr p);
    void searchIndexes(FeatureIndexType indexType);
    void searchRoot(DataPtr ppRoot);
    void searchBranch(DataPtr p);
    void searchLeaf(DataPtr p);
    void addResult(uint32_t item);

    Query* query_;
    uint32_t tipAndFlags_;
    FastFilterHint fastFilterHint_;     // TODO: initialize
    DataPtr pTile_;
    // MatcherMethod method_;      // TODO: not needed if we use a single method for all types
    QueryResults* results_;
};



