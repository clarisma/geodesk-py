// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "Filter.h"
#include "SpatialFilter.h"


Box Filter::getBounds() const
{
    return (flags_ & FilterFlags::USES_BBOX) ?
        reinterpret_cast<const SpatialFilter*>(this)->bounds() : Box::ofWorld();
}




