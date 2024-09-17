// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "View.h"

// TODO: how does this view handle its resources?

namespace geodesk::detail {

template <typename T>
class EmptyView : public View<T>
{
public:
    using View::View;

    virtual View<T> create(FeatureStore* store, uint32_t flags,
        FeatureTypes types, ViewContext context,
        const MatcherHolder* matcher, const Filter* filter)
    {
        return EmptyView<T>(store, flags, types, context, matcher, filter);
    }
    bool isEmpty() const override { return true; }
    uint64_t count() const override { return 0; }
    Query<T> query() const override { return EmptyQuery(); }
};

} // namespace geodesk::detail
