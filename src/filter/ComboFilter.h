// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "SpatialFilter.h"

class ComboFilter : public SpatialFilter
{
public:
    ComboFilter(const Filter* a, const Filter* b);
    ~ComboFilter();

    virtual bool accept(FeatureStore* store, FeatureRef feature, FastFilterHint fast) const;

private:
    void add(const Filter* f);

    std::vector<const Filter*> filters_;
};