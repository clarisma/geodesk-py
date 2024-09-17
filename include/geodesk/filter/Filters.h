// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <geodesk/feature/Feature.h>

class Filter;

namespace geodesk {

class Filters
{
public:
    static const Filter* intersects(Feature feature);
    static const Filter* within(Feature feature);
};

} // namespace geodesk::detail