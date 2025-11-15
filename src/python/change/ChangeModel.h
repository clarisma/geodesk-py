// Copyright (c) 2025 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include "PyChangedFeature.h"
#include <clarisma/data/HashMap.h>
#include <geodesk/geom/Coordinate.h>
#include <geodesk/geom/FixedLonLat.h>
#include "python/util/PythonRef.h"

using PyFeatureRef = PythonRef<PyChangedFeature>;

class ChangeModel
{
public:
    clarisma::HashMap<FixedLonLat, PyFeatureRef> createdAnonNodes;
    clarisma::HashMap<Coordinate, PyFeatureRef> existingAnonNodes;
    std::vector<PyFeatureRef> created[3];
    clarisma::HashMap<uint64_t, PyFeatureRef> existing[3];
    std::vector<PyFeatureRef> deleted[3];
};
