// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "QueryAdapter.h"
#include "query/AbstractQuery.h"

namespace geodesk::detail {

class EmptyQueryImpl : public AbstractQuery
{
public:
    EmptyQueryImpl(FeatureStore* store) :  AbstractQuery(store) {}
};

class EmptyQuery : public QueryAdapter<EmptyQueryImpl>
{
public:
    EmptyQuery(const View& view) : QueryAdapter(view.store())
    {
    }

    FeaturePtr next() override
    {   
        return FeaturePtr();
    }
};

} // namespace geodesk::detail
