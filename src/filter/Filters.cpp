// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include <geodesk/filter/Filters.h>
#include "filter/IntersectsFilter.h"
#include "filter/WithinFilter.h"
#include <geodesk/feature/QueryException.h>

namespace geodesk {

const Filter* filter(PreparedFilterFactory&& factory, Feature feature)
{
    const Filter* filter;
    if(feature.isAnonymousNode())
    {
        filter = factory.forCoordinate(feature.xy());
    }
    else
    {
        filter = factory.forFeature(feature.store(), feature.ptr());
    }
    if(filter == nullptr)
    {
        throw QueryException("Filter not implemented");
    }
    return filter;
}


const Filter* Filters::intersects(Feature feature)
{
    return filter(IntersectsFilterFactory(), feature);
}

const Filter* Filters::within(Feature feature)
{
    return filter(WithinFilterFactory(), feature);
}

} // namespace geodesk::detail