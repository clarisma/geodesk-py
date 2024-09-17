// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "feature/WayPtr.h"

class FeatureNodeIterator
{
public:
    FeatureNodeIterator(FeatureStore* store);
    FeatureStore* store() const { return store_; }
    void start(DataPtr pBody, int flags, const MatcherHolder* matcher, const Filter* filter);
    NodePtr next();

private:
    FeatureStore* store_;
    const MatcherHolder* matcher_;
    const Filter* filter_;
    Tip currentTip_;
    int32_t currentNode_;
    DataPtr p_;
    DataPtr pForeignTile_;
};
