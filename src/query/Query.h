// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "AbstractQuery.h"
#include <condition_variable>
#include <unordered_set>
#include "QueryResults.h"
#include "TileIndexWalker.h"
#include "feature/FeatureStore.h"
#include "geom/Box.h"
#include "match/Matcher.h"
#include <common/util/log.h>

class Filter;

// TODO: Maybe call this a "Cursor"

class Query : public AbstractQuery
{
public:
    Query(FeatureStore* store, const Box& box, FeatureTypes types, 
        const MatcherHolder* matcher, const Filter* filter); 
    ~Query();
    const Box& bounds() const { return tileIndexWalker_.bounds(); }
    const FeatureTypes types() const { return types_; }
    const MatcherHolder* matcher() const { return matcher_; }
    const Filter* filter() const { return filter_; }
    FeatureStore* store() const { return store_; }
    void offer(QueryResults* results);
    void cancel();

    FeaturePtr next();

    static const uint32_t REQUIRES_DEDUP = 0x8000'0000;

private:
    const QueryResults* take();
    void requestTiles();

    // static const int MAX_PENDING_TILES = 8;

    // FeatureStore* store_;  // moved to AbstractQuery
    FeatureTypes types_;
    const MatcherHolder* matcher_;
    const Filter* filter_;
    int32_t pendingTiles_;      // TODO: rearrange to avoid needless gaps
    const QueryResults* currentResults_;
    int32_t currentPos_;
    bool allTilesRequested_;
    std::unordered_set<uint64_t> potentialDupes_;
    TileIndexWalker tileIndexWalker_;

    // these are used by multiple threads:
    // TODO: padding to avoid false sharing
    // maybe use TileIndexWalker for padding as it has lots
    // of unused entries?

    std::mutex mutex_;
    std::condition_variable resultsReady_;
    QueryResults* queuedResults_;
    int32_t completedTiles_;
    bool isCancelled_;
};

