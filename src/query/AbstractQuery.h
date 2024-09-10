// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

class FeatureStore;

class AbstractQuery
{
public:
    AbstractQuery(FeatureStore* store) :
        store_(store)
    {
    }

    FeatureStore* store() const noexcept { return store_; }

protected:
    FeatureStore* store_;
};