// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "QueryAdapter.h"
#include "View.h"
#include "query/Query.h"

namespace geodesk::detail {

class WorldQuery : public QueryAdapter<Query>
{
public:
    WorldQuery(const View& view) :
        QueryAdapter(view.store(), view.bounds(), view.types(), view.matcher(), view.filter())
    {
    }

    ~WorldQuery() override
    {
        // TODO: await completion of query
    }

    FeaturePtr next() override
    {
        return FeaturePtr(query_.next());
    }
};

} // namespace geodesk::detail
