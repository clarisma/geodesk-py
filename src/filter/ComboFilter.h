// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "SpatialFilter.h"

class ComboFilter : public SpatialFilter
{
public:
    ComboFilter(const Filter* a, const Filter* b);
    ~ComboFilter();

    bool accept(FeatureStore* store, FeatureRef feature, FastFilterHint fast) const override;
    int acceptTile(Tile tile) const override;

private:
    void add(const Filter* f);

    std::vector<const Filter*> filters_;
};