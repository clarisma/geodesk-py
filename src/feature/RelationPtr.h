// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "FeaturePtr.h"

class RelationPtr : public FeaturePtr
{
public:
	explicit RelationPtr(FeaturePtr f) : FeaturePtr(f.ptr()) { assert(isNull() || isRelation()); }
	explicit RelationPtr(DataPtr p) : FeaturePtr(p) {}

	bool containsPointFast(double cx, double cy);
};
