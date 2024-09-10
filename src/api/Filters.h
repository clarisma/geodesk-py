// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "Feature.h"

class Filter;

// TODO: Move to geodesk::filter

namespace geodesk::detail {

using geodesk::Feature;

class Filters
{
public:
    static const Filter* intersects(Feature feature);
    static const Filter* within(Feature feature);
};

} // namespace geodesk::detail